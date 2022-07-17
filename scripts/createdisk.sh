#!/bin/sh

dd if=/dev/zero of=build/disk.img bs=512 count=2097152
sfdisk build/disk.img < scripts/layout.sfdisk

DISK_DEVICE=$(sudo losetup --find --show build/disk.img)
EFI_DEVICE="${DISK_DEVICE}p2"
ROOT_DEVICE="${DISK_DEVICE}p3"
EFI_MOUNTPOINT="build/mnt/efi"
ROOT_MOUNTPOINT="build/mnt/root"

sudo mkfs.fat -F 32 $EFI_DEVICE
sudo mkfs.ext2 $ROOT_DEVICE

mkdir -p $EFI_MOUNTPOINT
mkdir -p $ROOT_MOUNTPOINT
sudo mount $EFI_DEVICE $EFI_MOUNTPOINT
sudo mount $ROOT_DEVICE $ROOT_MOUNTPOINT

sudo grub-install --target=i386-pc --boot-directory="${EFI_MOUNTPOINT}/boot" $DISK_DEVICE
sudo grub-install --target=x86_64-efi --boot-directory="${EFI_MOUNTPOINT}/boot" --efi-directory=$EFI_MOUNTPOINT --removable

sync
sudo umount $EFI_MOUNTPOINT
sudo umount $ROOT_MOUNTPOINT
rmdir $EFI_MOUNTPOINT
rmdir $ROOT_MOUNTPOINT
sudo losetup --detach $DISK_DEVICE
