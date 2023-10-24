#!/bin/bash

function install_shinjuku()
{
    mkdir -p /tmp/shinjuku_kernel && pushd /tmp/shinjuku_kernel

    # Install the 4.4.185 kernel for Shinjuku
    wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v4.4.185/linux-headers-4.4.185-0404185_4.4.185-0404185.201907100448_all.deb
    wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v4.4.185/linux-headers-4.4.185-0404185-generic_4.4.185-0404185.201907100448_amd64.deb
    wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v4.4.185/linux-image-unsigned-4.4.185-0404185-generic_4.4.185-0404185.201907100448_amd64.deb
    wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v4.4.185/linux-modules-4.4.185-0404185-generic_4.4.185-0404185.201907100448_amd64.deb

    # Install the kernel
    sudo dpkg -i *.deb
    sudo update-grub

    popd
}


function install_persephone()
{
    mkdir -p /tmp/persephone_kernel && pushd /tmp/persephone_kernel

    wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v4.15/linux-headers-4.15.0-041500_4.15.0-041500.201802011154_all.deb
    wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v4.15/linux-headers-4.15.0-041500-generic_4.15.0-041500.201802011154_amd64.deb
    wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v4.15/linux-image-4.15.0-041500-generic_4.15.0-041500.201802011154_amd64.deb

    # Install the kernel
    sudo dpkg -i *.deb
    sudo update-grub

    popd
}

if [ "$1" == "shinjuku" ]; then
    echo "Installing kernel for Shinjuku"
    install_shinjuku

elif [ "$1" == "psp" ]; then
    echo "Installing kernel for Persephone"
    install_persephone
    
elif [ "$1" == "all" ]; then
    echo "Installing kernel for Shinjuku"
    install_shinjuku
    echo "Installing kernel for Persephone"
    install_persephone
else
    echo "Usage: $0 <shinjuku|persephone|all>"
    exit 1
fi




