SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

export CONCORD_DIR_PATH=$SCRIPT_DIR
export CONCORD_SRC_PATH=$CONCORD_DIR_PATH/src
export CONCORD_LIB_PATH=$CONCORD_DIR_PATH/src/lib
export CONCORD_TIMESTAMP_PATH=$CONCORD_DIR_PATH/src/lib/concord_timestamps.log
export CONCORD_RDTSC_PASS=$CONCORD_SRC_PASS/rdtsc-pass/build/src/libConcordPass.so
export CONCORD_CACHE_LINE_PASS=$CONCORD_SRC_PASS/cache-line-pass/build/src/libConcordPass.so
export CNC_LLC=llc-9