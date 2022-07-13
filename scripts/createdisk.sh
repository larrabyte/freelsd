#!/bin/sh

# --------------------------------------------------------------
# createdisk.sh: creates a disk image with GRUB installed on it.
# --------------------------------------------------------------

dd if=/dev/zero of=build/disk.img bs=512 count=131072
sfdisk build/disk.img < scripts/layout.sfdisk

loopdisk=$(sudo losetup --find --show build/disk.img)
loopgrub=$(sudo losetup --find --show build/disk.img -o 1048576)

sudo mkdosfs -F32 -f 2 $loopgrub
mkdir -p build/mnt
sudo mount $loopgrub build/mnt
sudo grub-install --root-directory=$(pwd)/build/mnt --no-floppy --modules="normal part_msdos ext2 multiboot" $loopdisk
sudo umount build/mnt
sudo losetup --detach $loopdisk $loopgrub
