#!/bin/bash

QUANTUMS=(1000 2500 5000 10000 15000 20000 25000 50000 100000)
BENCHS=(0 1 2 3)

rm -f log.txt
rm -f fig2.txt

for bench in "${BENCHS[@]}"; do
    for quantum in "${QUANTUMS[@]}"; do
        make clean

        make BENCH=$bench SCHED_QUANTUM=$quantum

        output=$(sudo ./fig2)

        echo "$bench $quantum $output" >> log.txt

        num_packets=$(echo $output | grep -oP '(?<=Packets processed: )\d+')

        echo "$bench $quantum $num_packets" >> fig2.txt
    done
done

python3 plot.py