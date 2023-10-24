#!/bin/bash

source ../../env.sh

pushd ${CONCORD_DIR_PATH}/benchmarks/overhead

sudo bash setup.sh
bash run.sh
python3 generate_table.py > $CONCORD_DIR_PATH/figures/table1/table1.csv

popd