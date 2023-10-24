## Reproducing Figures

### Table 1: Overhead and Timeliness

Table 1 is about overhead and timeliness of Concord’s instrumentation. Baseline is Compiler-Interrupts. This benchmark will install three different benchmarking suite: phoenix, parsec, splash2. 

- Time needed to get the table: ~ 1 hour

```sh
cd table1/
bash table1.sh
```

See the output in `table1/table1.csv`


### Figure 1: Abstract visualization

```sh
cd fig1/
bash fig1.sh
```

See the output in `fig1/fig1.eps`

### Figure 2: Overhead of preemption mechanisms

This benchmark will install Dune, please make sure you have run this benchmark on a server machine with Ubuntu 18.04 and kernel version 4.4.185-0404185-generic.

If you are using a different kernel version, you can change the kernel version through `scripts`:

```sh
cd scripts
bash download_kernel.sh shinjuku
bash boot_kernel.sh shinjuku
```

Machine will be rebooted after please check the kernel version with `uname -r`. After that, you can run the benchmark:

```sh
cd fig2/
bash fig2.sh
```

See the output in `fig2/fig2.png`


### Figure 5: The impact of non-instantaneous preemption

```sh
cd fig5/
bash fig5.sh
```

See the output in `fig5/fig5.eps`
