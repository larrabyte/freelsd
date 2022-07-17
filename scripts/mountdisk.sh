#!/bin/sh

EFI_MOUNTPOINT="build/mnt/efi"
ROOT_MOUNTPOINT="build/mnt/root"

if [[ ! -f build/disk.img ]]; then
    printf "No disk image present.\n"
    exit -1
fi

mkdir -p $EFI_MOUNTPOINT
mkdir -p $ROOT_MOUNTPOINT

DISK_DEVICE=$(sudo losetup --find --show build/disk.img)
EFI_DEVICE="${DISK_DEVICE}p2"
ROOT_DEVICE="${DISK_DEVICE}p3"
sudo mount $EFI_DEVICE $EFI_MOUNTPOINT
sudo mount $ROOT_DEVICE $ROOT_MOUNTPOINT

if [[ "$1" == "install" ]]; then
    GRUB_CFG_DIRECTORY="${EFI_MOUNTPOINT}/boot/grub"
    KERNEL_DIRECTORY="${ROOT_MOUNTPOINT}/boot"
    sudo mkdir -p $GRUB_CFG_DIRECTORY
    sudo mkdir -p $KERNEL_DIRECTORY

    sudo cp build/grub.cfg "${GRUB_CFG_DIRECTORY}/grub.cfg"
    sudo cp build/kernel.elf "${KERNEL_DIRECTORY}/kernel.elf"

    sync
    sudo umount $EFI_MOUNTPOINT
    sudo umount $ROOT_MOUNTPOINT
    rmdir $EFI_MOUNTPOINT
    rmdir $ROOT_MOUNTPOINT

    sudo losetup --detach $DISK_DEVICE
fi
