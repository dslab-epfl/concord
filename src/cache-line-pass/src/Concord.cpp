#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/IVDescriptors.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"

#include <algorithm>
#include <cxxabi.h>
#include <math.h>
#include <queue>
#include <string>

using namespace llvm;

bool DEBUG_MODE = false;

std::vector<BasicBlock*> nmodBasicBlocks = {};
std::vector<std::string> visitedFunctions = {};
std::vector<Loop*> visitedLoops = {};

static cl::opt<unsigned> ci_loopBodyUnrollSize("loop_body_size", cl::desc("Loop body unroll size in number of LLVM instructions"), cl::value_desc("unroll size"), cl::init(0));
static cl::opt<int> ci_enableInstrumentation("c_instrument",
                                             cl::desc("Enable instrumentation"),
                                             cl::value_desc("enable instrumentation"),
                                             cl::init(1));
static cl::opt<int> ci_modifiedSubLoops("modified_subloops",
                                        cl::desc("Number of modified subloops"),
                                        cl::value_desc("modified subloops"),
                                        cl::init(200));
static cl::opt<int> ci_disableBoundedLoops("disable_bounded_loops",
                                           cl::desc("Disable bounded loops"),
                                           cl::value_desc("disable bounded loops"),
                                           cl::init(0));

namespace
{
  struct ConcordPass : public ModulePass
  {
    static char ID;
    long loopCount = 0;
    long loopBodyUnrollSize = 0;
    int enableInstrumentation = 1;
    long modifiedSubLoops = 0;
    int disableBoundedLoops = 1;

    std::set<Function*> annotFuncs;

    ConcordPass() : ModulePass(ID) {}

    void getAnalysisUsage(AnalysisUsage& AU) const override
    {
      AU.addRequired<ScalarEvolutionWrapperPass>();
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
      AU.addRequired<PostDominatorTreeWrapperPass>();
      AU.addRequired<CallGraphWrapperPass>();
      AU.addPreserved<CallGraphWrapperPass>();
    }

    virtual bool doInitialization(Module& M) override
    {
      getAnnotatedFunctions(&M);
      return false;
    }

    void getAnnotatedFunctions(Module* M)
    {
      for (Module::global_iterator I = M->global_begin(), E = M->global_end(); I != E; ++I)
      {
        if (I->getName() == "llvm.global.annotations")
        {
          ConstantArray* CA = dyn_cast<ConstantArray>(I->getOperand(0));
          for (auto OI = CA->op_begin(); OI != CA->op_end(); ++OI)
          {
            ConstantStruct* CS = dyn_cast<ConstantStruct>(OI->get());
            Function* FUNC = dyn_cast<Function>(CS->getOperand(0)->getOperand(0));
            GlobalVariable* AnnotationGL = dyn_cast<GlobalVariable>(CS->getOperand(1)->getOperand(0));
            StringRef annotation = dyn_cast<ConstantDataArray>(AnnotationGL->getInitializer())->getAsCString();
            if (annotation == "concord_skip")
            {
              annotFuncs.insert(FUNC);
            }
          }
        }
      }
    }

    virtual bool runOnModule(Module& M)
    {
      loopBodyUnrollSize = ci_loopBodyUnrollSize;
      enableInstrumentation = ci_enableInstrumentation;
      modifiedSubLoops = ci_modifiedSubLoops;
      disableBoundedLoops = ci_disableBoundedLoops;

      errs() << "========== Starting the analysis of the module ========== \n";
      errs() << "> Module Name: " << M.getName() << "\n";
      errs() << "> Enable instrumentation: " << enableInstrumentation << "\n";
      errs() << "> Size of each loop after unrolling: " << loopBodyUnrollSize << "\n";
      errs() << "> Modified subloops: " << modifiedSubLoops << "\n";
      errs() << "> Disable bounded loops: " << disableBoundedLoops << "\n";

      if(enableInstrumentation)
      {
        std::vector<Loop*> loops;

        for (auto& F : M)
        {
          
          // Check annotations
          if (shouldInstrumentFunc(F))
          {
            errs() << "> Skipping function: '" << F.getName()
                  << "' because it has been annotated with concord_skip attribute.\n";
            continue;
          }
          // External function we can skip it ===> like malloc, stdlib funcs
          if (F.isDeclaration())
          {
            errs() << "> This is declaration, skip " << F.getName() << "\n";
            continue;
          }

          std::string funcName = F.getName().str();
          std::string demangledFuncName = funcName;

          int status;
          char* demangled = abi::__cxa_demangle(demangledFuncName.c_str(), 0, 0, &status);
          if (status == 0)
          {
            demangledFuncName = demangled;
          }

          if (std::find(visitedFunctions.begin(), visitedFunctions.end(), demangledFuncName) != visitedFunctions.end())
          {
            continue;
          }

          visitedFunctions.push_back(demangledFuncName);

          // Instrument function
          // instrumentFunction(F, M, demangledFuncName);

          // Get the loop info
          LoopInfo& LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
          auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();

          for (Loop* loop : LI) {
            int subLoopCounter = 0;

            std::queue<Loop*> worklist;
            worklist.push(loop);

            while (!worklist.empty()) {
              Loop* currentLoop = worklist.front();

              int cnt = loopBodyUnrollSize / estimateLoopBodySize(currentLoop, LI);

              UnrollLoopOptions ULO{
                .AllowRuntime = true,
                .AllowExpensiveTripCount = true,
                .UnrollRemainder = true,
              };
              ULO.Count = cnt;

              AssumptionCache* AC = &getAnalysis<AssumptionCacheTracker>(F).getAssumptionCache(F);

              LoopUnrollResult result = UnrollLoop(currentLoop, ULO, &LI, &SE, &getAnalysis<DominatorTreeWrapperPass>(F).getDomTree(), AC, nullptr, true);

              if (result == LoopUnrollResult::PartiallyUnrolled) {
                errs() << "Loop partially unrolled \n";
              }
              else if (result == LoopUnrollResult::FullyUnrolled) {
                errs() << "Loop fully unrolled \n";
              }
              else {
                errs() << "Loop not unrolled \n";
              }

              worklist.pop();

              if (SE.getSmallConstantTripCount(currentLoop) > 0 && disableBoundedLoops) {
                continue;
              }

              instrumentLoop(currentLoop, F, M, LI, demangledFuncName);

              if (modifiedSubLoops != 0) {
                for (Loop::iterator SL = currentLoop->begin(), SLEnd = currentLoop->end(); SL != SLEnd; ++SL) {
                  if (subLoopCounter < modifiedSubLoops) {
                    worklist.push(*SL);
                    subLoopCounter++;
                  }
                }
              }
            }
          }
        }
        errs() << "> Unique loops: " << loops.size() << "\n";
      }
      errs() << "========== Finished the analysis of the module ========== \n";
      errs() << "========== Instrumented with Cache Line Pass ========== \n";
      return true;
    }

    bool shouldInstrumentFunc(Function& F)
    {
      return annotFuncs.find(&F) != annotFuncs.end();
    }

    bool instrumentLoop(Loop* loop, Function& F, Module& M, LoopInfo& LI, std::string demangledFuncName)
    {
      errs() << "Instrumenting loop in function: " << demangledFuncName << "\n";

      BasicBlock* entryBB = loop->getHeader();
      Instruction* firstInst = &*entryBB->getFirstInsertionPt();

      IRBuilder<> builder(firstInst);

      GlobalVariable* preemptNow = M.getGlobalVariable("concord_preempt_now");

      if (!preemptNow)
      {
        preemptNow = new GlobalVariable(M,
                                        Type::getInt32Ty(M.getContext()),
                                        false,
                                        GlobalValue::ExternalLinkage,
                                        0,
                                        "concord_preempt_now",
                                        0,
                                        GlobalValue::GeneralDynamicTLSModel,
                                        0);
      }

      LoadInst* loadPreemptNow = builder.CreateLoad(preemptNow);
      ConstantInt* preemptNowValue = ConstantInt::get(Type::getInt32Ty(M.getContext()), 1, false);

      Value* condition = builder.CreateICmpEQ(loadPreemptNow, preemptNowValue, "concord_preempt_now_condition");

      Instruction* i = SplitBlockAndInsertIfThen(condition,
                                                 firstInst,
                                                 false,
                                                 nullptr,
                                                 &getAnalysis<DominatorTreeWrapperPass>(F).getDomTree(),
                                                 &LI);

      // // // Add branch weight further optimization
      // // // https://llvm.org/docs/BranchWeightMetadata.html
      BranchInst* br = dyn_cast<BranchInst>(entryBB->getTerminator());
      br->setMetadata("branch_weights", MDBuilder(M.getContext()).createBranchWeights(1, 10000));

      builder.SetInsertPoint(i);
      i->getParent()->setName("if_clock_fired");
      Function* concordFunc = M.getFunction("concord_func");

      if (!concordFunc)
      {
        FunctionType* FuncTy = FunctionType::get(IntegerType::get(M.getContext(), 32), true);

        concordFunc = Function::Create(FuncTy, GlobalValue::ExternalLinkage, "concord_func", M);
        concordFunc->setCallingConv(CallingConv::C);
      }

      builder.CreateCall(concordFunc, {});

      // errs() << "!+ " << demangledFuncName << " *'* " << estimatedInstCc << "\n";
      // errs() << "Loop instrumented \n";
      return false;
    }

    uint32_t estimateLoopBodySize(Loop* loop, LoopInfo& LI)
    {
      uint32_t estimatedInstCc = 0;
      for (Loop::block_iterator BI = loop->block_begin(), BE = loop->block_end(); BI != BE; ++BI)
      {
        BasicBlock* BB = *BI;
        for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ++II)
        {
          estimatedInstCc++;
        }
      }
      return estimatedInstCc;
    }


    bool instrumentFunction(Function& F, Module& M, std::string demangledFuncName)
    {
      errs() << "Instrumenting function: " << demangledFuncName << "\n";

      Instruction* firstInst = &*F.getEntryBlock().getFirstInsertionPt();

      IRBuilder<> builder(firstInst);

      GlobalVariable* preemptNow = M.getGlobalVariable("concord_preempt_now");

      if (!preemptNow)
      {
        preemptNow = new GlobalVariable(M,
                                        Type::getInt32Ty(M.getContext()),
                                        false,
                                        GlobalValue::ExternalLinkage,
                                        0,
                                        "concord_preempt_now",
                                        0,
                                        GlobalValue::GeneralDynamicTLSModel,
                                        0);
      }

      LoadInst* loadPreemptNow = builder.CreateLoad(preemptNow);
      ConstantInt* preemptNowValue = ConstantInt::get(Type::getInt32Ty(M.getContext()), 1, false);

      Value* condition = builder.CreateICmpEQ(loadPreemptNow, preemptNowValue, "concord_preempt_now_condition");

      Instruction* i = SplitBlockAndInsertIfThen(condition,
                                                 firstInst,
                                                 false,
                                                 nullptr,
                                                 &getAnalysis<DominatorTreeWrapperPass>(F).getDomTree());

      // // // Add branch weight further optimization
      // // // https://llvm.org/docs/BranchWeightMetadata.html
      BranchInst* br = dyn_cast<BranchInst>(F.getEntryBlock().getTerminator());
      br->setMetadata("branch_weights", MDBuilder(M.getContext()).createBranchWeights(1, 10000));

      builder.SetInsertPoint(i);
      i->getParent()->setName("if_clock_fired");
      Function* concordFunc = M.getFunction("concord_func");

      if (!concordFunc)
      {
        FunctionType* FuncTy = FunctionType::get(IntegerType::get(M.getContext(), 32), true);

        concordFunc = Function::Create(FuncTy, GlobalValue::ExternalLinkage, "concord_func", M);
        concordFunc->setCallingConv(CallingConv::C);
      }

      builder.CreateCall(concordFunc, {});

      // errs() << "!+ " << demangledFuncName << " *'* " << estimatedInstCc << "\n";
      errs() << "Function instrumented \n";
      return true;
    }
  };
} // namespace

char ConcordPass::ID = 0;
static RegisterPass<ConcordPass> Y("yield", "Concord Pass", true, false);

static void registerConcordPass(const PassManagerBuilder&, legacy::PassManagerBase& PM)
{
  PM.add(new ConcordPass());
}