# Concord: An efficient runtime for microsecond-scale applications

This repository contains the source code for Concord, which was published at [SOSP'23](https://dslab.epfl.ch/pubs/concord.pdf).
In particular, it contains a) the two LLVM passes used by Concord to automatically instrument applications and b) the implementation of the Concord runtime on the ([Shinjuku](https://github.com/stanford-mast/shinjuku) and dataplane OS.

## Organization

Subdirectories have their own README files.

* `benchmarks` - The different applications we tested Concord on and overhead measurements.
* `concord-shinjuku` - The Concord runtime integrated into the Shinjuku dataplane OS. 
* `figures` - Scripts to generate the figures in the paper
* `scripts` - General scripts to run concord
* `schedsim` - Our queueing simulator
* `src` - LLVM passes and the Concord runtime library

## Getting started

Clone the repository, please make sure you have cloned via ssh, otherwise the submodules will not be cloned.

```sh
git clone --recurse-submodules git@github.com:dslab-epfl/concord.git
cd concord
```

Then setup Concord, this script will build concord-cache-line, concord-rdtsc pass, and install the required dependencies.

```sh
./setup.sh
```


## Simple Examples
First run the basic examples to see cache-line and rdtsc based instrumentation in action. 

```sh
cd benchmarks/basics
make
```

See the instrumentation outputs:
```
> Module Name: <stdin>
> Enable instrumentation: 1
> Modified subloops: 200
> Disable bounded loops: 0
Instrumenting loop in function: hello
Instrumenting loop in function: main
> This is declaration, skip atoi
> This is declaration, skip puts
> Unique loops: 0
```

After run the programs, first run cache-line based instrumentation:

```sh
./hello.out

# Program Modified !! 
# Hello, world!
```

Then run rdtsc based instrumentation, it will print the number of cycles between two `concord_rdtsc_func` calls.

```sh
./hello_rdtsc.out
# 16070
# 16054
# 16060
# 16056
# 16060
# 16056
# 16060
# 16056
# 16060
# ...
```

To run more complex examples, including the entire runtime on Shinjuku, please refer to the README within the `concord-shinjuku` directory. 
