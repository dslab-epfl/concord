SCRIPT_PATH=$(dirname $(readlink -f $0))

build_leveldb()
{
    pushd $SCRIPT_PATH/leveldb
    make -f concord.mk all -j3
    popd
}

build_wrapper()
{
    pushd lib
    echo "========= Building wrapper ========="
    make

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SCRIPT_PATH

    echo "========= Running tests ========="
    ./test.o
    ./test_ubench.o
    ./test_clear.o
    ./test_rdtsc.o
    ./dl_test.o ./concord_apileveldb_rdtsc.so
    popd
}

if [ "$1" = "leveldb" ]; then
    build_leveldb
elif [ "$1" = "wrapper" ]; then
    build_wrapper
else
    build_leveldb
    build_wrapper
fi
