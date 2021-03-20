# --------------------------------------------------------------
# createdisk.sh: creates a disk image with GRUB installed on it.
# --------------------------------------------------------------

dd if=/dev/zero of=build/disk.img bs=512 count=131072
sfdisk build/disk.img < scripts/layout.sfdisk

loopdisk=$(losetup --find --show build/disk.img)
loopgrub=$(losetup --find --show build/disk.img -o 1048576)

mkdosfs -F32 -f 2 $loopgrub
mkdir -p build/mnt
mount $loopgrub build/mnt
grub-install --root-directory=$(pwd)/build/mnt --no-floppy --modules="normal part_msdos ext2 multiboot" $loopdisk
umount build/mnt
rmdir build/mnt
losetup --detach $loopdisk $loopgrub
