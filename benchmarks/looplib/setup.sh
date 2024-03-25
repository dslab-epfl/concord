SCRIPT_PATH=$(dirname $(readlink -f $0))


build_wrapper()
{
    pushd lib
    echo "========= Building wrapper ========="
    make

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SCRIPT_PATH

    echo "========= Running tests ========="
    ./test.o
    ./test_ubench.o
    popd
}

build_wrapper