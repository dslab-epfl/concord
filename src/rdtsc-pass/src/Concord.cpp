#include <cxxabi.h>
#include <math.h>

#include <algorithm>
#include <queue>
#include <string>

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CodeMetrics.h"
#include "llvm/Analysis/IVDescriptors.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/SimplifyIndVar.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

using namespace llvm;

std::vector<BasicBlock *> nmodBasicBlocks = {};
std::vector<std::string> visitedFunctions = {};
std::vector<Loop *> visitedLoops = {};

bool unrollLoop(Loop *L, unsigned Count, unsigned Threshold, LoopInfo *LI, DominatorTree *DT, ScalarEvolution *SE,
                AssumptionCache *AC, const TargetTransformInfo &TTI);

static cl::opt<long> ci_loopBodyUnrollSize("unroll", cl::desc("Loop body unroll size"), cl::value_desc("unroll size"),
                                           cl::init(0));
static cl::opt<int> ci_enableInstrumentation("c_instrument", cl::desc("Enable instrumentation"),
                                             cl::value_desc("enable instrumentation"), cl::init(1));
static cl::opt<int> ci_modifiedSubLoops("modified_subloops", cl::desc("Number of modified subloops"),
                                        cl::value_desc("modified subloops"), cl::init(200));
static cl::opt<int> ci_disableBoundedLoops("disable_bounded_loops", cl::desc("Disable bounded loops"),
                                           cl::value_desc("disable bounded loops"), cl::init(0));

namespace {
struct ConcordPass : public ModulePass {
    static char ID;
    long loopCount = 0;
    int enableInstrumentation = 1;
    long loopBodyUnrollSize = 0;
    long modifiedSubLoops = 0;
    int disableBoundedLoops = 1;

    std::set<Function *> annotFuncs;

    ConcordPass() : ModulePass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.addRequired<ScalarEvolutionWrapperPass>();
        AU.addRequired<DominatorTreeWrapperPass>();
        AU.addRequired<LoopInfoWrapperPass>();
        AU.addRequired<PostDominatorTreeWrapperPass>();
        AU.addRequired<CallGraphWrapperPass>();
        AU.addRequired<AssumptionCacheTracker>();
        AU.addRequired<TargetTransformInfoWrapperPass>();
        getLoopAnalysisUsage(AU);
    }

    virtual bool doInitialization(Module &M) override {
        getAnnotatedFunctions(&M);
        return false;
    }

    void getAnnotatedFunctions(Module *M) {
        for (Module::global_iterator I = M->global_begin(), E = M->global_end(); I != E; ++I) {
            if (I->getName() == "llvm.global.annotations") {
                ConstantArray *CA = dyn_cast<ConstantArray>(I->getOperand(0));
                for (auto OI = CA->op_begin(); OI != CA->op_end(); ++OI) {
                    ConstantStruct *CS = dyn_cast<ConstantStruct>(OI->get());
                    Function *FUNC = dyn_cast<Function>(CS->getOperand(0)->getOperand(0));
                    GlobalVariable *AnnotationGL = dyn_cast<GlobalVariable>(CS->getOperand(1)->getOperand(0));
                    StringRef annotation = dyn_cast<ConstantDataArray>(AnnotationGL->getInitializer())->getAsCString();
                    if (annotation == "concord_skip") {
                        annotFuncs.insert(FUNC);
                    }
                }
            }
        }
    }

    virtual bool runOnModule(Module &M) {
        loopBodyUnrollSize = ci_loopBodyUnrollSize;
        enableInstrumentation = ci_enableInstrumentation;
        modifiedSubLoops = ci_modifiedSubLoops;
        disableBoundedLoops = ci_disableBoundedLoops;

        errs() << "========== Starting the analysis of the module ========== \n";
        errs() << "> RDTSC Pass 1\n";
        errs() << "> Module Name: " << M.getName() << "\n";
        errs() << "> Enable instrumentation: " << enableInstrumentation << "\n";
        errs() << "> Modified subloops: " << modifiedSubLoops << "\n";
        errs() << "> Disable bounded loops: " << disableBoundedLoops << "\n";

        std::vector<Loop *> loops;

        for (auto &F : M) {
            // Check annotations
            if (shouldInstrumentFunc(F)) {
                errs() << "> Skipping function: '" << F.getName()
                       << "' because it has been annotated with concord_skip attribute.\n";
                continue;
            }

            // External function we can skip it ===> like malloc, stdlib funcs
            if (F.isDeclaration()) {
                errs() << "> This is declaration, skip " << F.getName() << "\n";
                continue;
            }

            std::string funcName = F.getName().str();
            std::string demangledFuncName = funcName;

            int status;
            char *demangled = abi::__cxa_demangle(demangledFuncName.c_str(), 0, 0, &status);
            if (status == 0) {
                demangledFuncName = demangled;
            }

            if (std::find(visitedFunctions.begin(), visitedFunctions.end(), demangledFuncName) !=
                visitedFunctions.end()) {
                continue;
            }

            visitedFunctions.push_back(demangledFuncName);

            // Get the loop info
            LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
            auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();

            // First unroll the loops
            for (Loop *loop : LI) {
                if (SE.getSmallConstantTripCount(loop) > 0 && disableBoundedLoops) {
                    continue;
                }

                std::queue<Loop *> worklist;
                worklist.push(loop);
                int subLoopCounter = 0;

                while (!worklist.empty()) {
                    Loop *currentLoop = worklist.front();
                    worklist.pop();

                    instrumentLoop(currentLoop, F, M, LI, demangledFuncName);

                    if (modifiedSubLoops == 0) {
                        continue;
                    }

                    for (Loop::iterator SL = currentLoop->begin(), SLEnd = currentLoop->end(); SL != SLEnd; ++SL) {
                        if (subLoopCounter < modifiedSubLoops) {
                            worklist.push(*SL);
                            subLoopCounter++;
                        }
                    }
                }
            }
        }

        errs() << "> Unique loops: " << loops.size() << "\n";

        errs() << "========== Finished the analysis of the module ========== \n";
        errs() << "========== Instrumented with RDTSC Pass ========== \n";
        return true;
    }

    bool shouldInstrumentFunc(Function &F) { return annotFuncs.find(&F) != annotFuncs.end(); }

    bool instrumentLoop(Loop *loop, Function &F, Module &M, LoopInfo &LI, std::string demangledFuncName) {
        errs() << "Instrumenting loop in function: " << demangledFuncName << "\n";

        BasicBlock *entryBB = loop->getHeader();
        Instruction *firstInst = &*entryBB->getFirstInsertionPt();

        IRBuilder<> builder(firstInst);

        GlobalVariable *concord_start_time = M.getGlobalVariable("concord_start_time");
        GlobalVariable *concord_preempt_after_cycle = M.getGlobalVariable("concord_preempt_after_cycle");

        if (!concord_start_time) {
            concord_start_time =
                new GlobalVariable(M, Type::getInt64Ty(M.getContext()), false, GlobalValue::ExternalLinkage, 0,
                                   "concord_start_time", 0, GlobalValue::GeneralDynamicTLSModel, 0);
        }

        if (!concord_preempt_after_cycle) {
            concord_preempt_after_cycle =
                new GlobalVariable(M, Type::getInt64Ty(M.getContext()), false, GlobalValue::ExternalLinkage, 0,
                                   "concord_preempt_after_cycle", 0, GlobalValue::GeneralDynamicTLSModel, 0);
        }

        LoadInst *loadPreemptTime = builder.CreateLoad(concord_start_time);
        LoadInst *loadPreemptAfterCycle = builder.CreateLoad(concord_preempt_after_cycle);

        CallInst *timeNow = builder.CreateIntrinsic(Intrinsic::readcyclecounter, {}, {}, nullptr, "time_now");

        // (rdtsc() - start_time) > preempt_after_cycle
        // rdtsc_result - start_time
        Value *diffTime = builder.CreateSub(timeNow, loadPreemptTime, "diff_time");
        Value *condition = builder.CreateICmpUGE(diffTime, loadPreemptAfterCycle, "concord_preempt_now_condition");
        Instruction *i = SplitBlockAndInsertIfThen(condition, firstInst, false, nullptr,
                                                   &getAnalysis<DominatorTreeWrapperPass>(F).getDomTree(), &LI);

        // // // Add branch weight further optimization
        // // // https://llvm.org/docs/BranchWeightMetadata.html
        BranchInst *br = dyn_cast<BranchInst>(entryBB->getTerminator());
        br->setMetadata("branch_weights", MDBuilder(M.getContext()).createBranchWeights(1, 10000));

        builder.SetInsertPoint(i);
        i->getParent()->setName("if_clock_fired");
        Function *concordFunc = M.getFunction("concord_rdtsc_func");

        if (!concordFunc) {
            FunctionType *FuncTy = FunctionType::get(IntegerType::get(M.getContext(), 32), true);

            concordFunc = Function::Create(FuncTy, GlobalValue::ExternalLinkage, "concord_rdtsc_func", M);
            concordFunc->setCallingConv(CallingConv::C);
        }

        builder.CreateCall(concordFunc, {});

        errs() << "Loop instrumented \n";
        return false;
    }

    bool instrumentFunction(Function &F, Module &M, std::string demangledFuncName) {
        errs() << "Instrumenting function: " << demangledFuncName << "\n";

        BasicBlock &entryBB = F.getEntryBlock();
        Instruction *firstInst = &*entryBB.getFirstInsertionPt();

        IRBuilder<> builder(firstInst);

        // get when the job started
        GlobalVariable *concord_start_time = M.getGlobalVariable("concord_start_time");
        GlobalVariable *concord_preempt_after_cycle = M.getGlobalVariable("concord_preempt_after_cycle");

        if (!concord_start_time) {
            concord_start_time =
                new GlobalVariable(M, Type::getInt64Ty(M.getContext()), false, GlobalValue::ExternalLinkage, 0,
                                   "concord_start_time", 0, GlobalValue::GeneralDynamicTLSModel, 0);
        }

        if (!concord_preempt_after_cycle) {
            concord_preempt_after_cycle =
                new GlobalVariable(M, Type::getInt64Ty(M.getContext()), false, GlobalValue::ExternalLinkage, 0,
                                   "concord_preempt_after_cycle", 0, GlobalValue::GeneralDynamicTLSModel, 0);
        }

        LoadInst *loadPreemptTime = builder.CreateLoad(concord_start_time);
        LoadInst *loadPreemptAfterCycle = builder.CreateLoad(concord_preempt_after_cycle);

        CallInst *timeNow = builder.CreateIntrinsic(Intrinsic::readcyclecounter, {}, {}, nullptr, "time_now");

        // (rdtsc() - start_time) > preempt_after_cycle
        // rdtsc_result - start_time
        Value *diffTime = builder.CreateSub(timeNow, loadPreemptTime, "diff_time");
        Value *condition = builder.CreateICmpUGE(diffTime, loadPreemptAfterCycle, "concord_preempt_now_condition");
        Instruction *i = SplitBlockAndInsertIfThen(condition, firstInst, false, nullptr,
                                                   &getAnalysis<DominatorTreeWrapperPass>(F).getDomTree());

        // // // Add branch weight further optimization
        // // // https://llvm.org/docs/BranchWeightMetadata.html
        BranchInst *br = dyn_cast<BranchInst>(entryBB.getTerminator());
        br->setMetadata("branch_weights", MDBuilder(M.getContext()).createBranchWeights(1, 10000));

        builder.SetInsertPoint(i);
        i->getParent()->setName("if_clock_fired");
        Function *concordFunc = M.getFunction("concord_rdtsc_func");

        if (!concordFunc) {
            FunctionType *FuncTy = FunctionType::get(IntegerType::get(M.getContext(), 32), true);

            concordFunc = Function::Create(FuncTy, GlobalValue::ExternalLinkage, "concord_rdtsc_func", M);
            concordFunc->setCallingConv(CallingConv::C);
        }

        builder.CreateCall(concordFunc, {});

        errs() << "Function instrumented \n";
        return true;
    }
};
};  // namespace

char ConcordPass::ID = 0;

static RegisterPass<ConcordPass> Y("yield", "Concord Pass", true, false);

static void registerConcordPass(const PassManagerBuilder &, legacy::PassManagerBase &PM)

{
    PM.add(new ConcordPass());
}
