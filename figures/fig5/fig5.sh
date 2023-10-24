#!/bin/bash -x

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
CONCORD_DIR="$(realpath -e "$SCRIPT_DIR"/../..)"

sudo apt update -y
curl -LO https://go.dev/dl/go1.17beta1.linux-amd64.tar.gz
sudo tar -C /usr/local -xzf go1.17beta1.linux-amd64.tar.gz
rm go1.17beta1.linux-amd64.tar.gz
export GOROOT=/usr/local/go
export PATH=$GOROOT/bin:$PATH


sudo apt install python3-pip -y
pip3 install fire pandas

pushd $CONCORD_DIR
    pushd schedsim
        go build
        export PATH=$CONCORD_DIR/schedsim:$PATH
        rm *.txt *.csv *.temp *.eps
        bash run.sh mb995
        cut -f1 single_queue.csv > common.temp
        cut -f5 single_queue.csv > sq.temp
        cut -f5 perfect_preemption.csv > pp.temp
        cut -f5 preemption_stddev1.csv > p1.temp
        cut -f5 preemption_stddev2.csv > p2.temp
        paste <(sed 's/\s$//' common.temp) <(sed 's/\s$//' sq.temp) <(sed 's/\s$//' pp.temp) <(sed 's/\s$//' p1.temp) <(sed 's/\s$//' p2.temp) > final.csv
        python3 scripts/plot.py
        mv fig5.eps $SCRIPT_DIR/
    popd
popd
