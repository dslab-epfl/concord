#!/bin/bash

boot() {
    kernlist="$(grep -i "menuentry '" /boot/grub/grub.cfg|sed -r "s|--class .*$||g")"
    printf "%s$kernlist"
    menuline="$(printf "%s$kernlist\n"|grep -ne $kernel | grep -v recovery | cut -f1 -d":")"
    menunum="$(($menuline-2))"
    echo $menunum
    sudo grub-reboot "1>$menunum"

    sudo cp /etc/default/grub /etc/default/grub.bak

    if grep -q "nox2apic" /etc/default/grub; then
        echo "nox2apic already present"
    else
        sudo sed -i 's/GRUB_CMDLINE_LINUX_DEFAULT="/GRUB_CMDLINE_LINUX_DEFAULT="nox2apic /' /etc/default/grub
    fi

    sudo reboot
}

if [ "$1" == "shinjuku" ]; then
    echo "Installing kernel for Shinjuku"
    kernel="4.4.185"

elif [ "$1" == "persephone" ]; then
    echo "Installing kernel for Persephone"
    kernel="4.15.0"
else
    echo "Usage: $0 <shinjuku|persephone|all>"
    exit 1
fi

boot