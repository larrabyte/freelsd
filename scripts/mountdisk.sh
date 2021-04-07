# ---------------------------------------------------------------
# mountdisk.sh: Automatically mounts build/disk.img to build/mnt.
# ---------------------------------------------------------------

platform=$(uname)
mkdir -p build/mnt

if [[ ! -z "$(ls -A build/mnt)" || ! $(id -u) = 0 && "$platform" == "Linux" ]]; then
    printf "[mkdisk] cannot run without root.\n"
    exit 1
fi

if [[ "$platform" == "Linux" ]]; then
    loopback=$(losetup --find --show build/disk.img -o 1048576)
    mount $loopback build/mnt -o umask=000
elif [[ "$platform" == "Darwin" ]]; then
    device=$(hdiutil attach -nomount build/disk.img | awk '/FAT/ {print $1}')
    mount -t msdos $device build/mnt > /dev/null
fi

printf "[mkdisk] mount successful.\n"

if [[ "$1" == "install" ]]; then
    cp build/grub.cfg build/mnt/boot/grub/grub.cfg
	cp build/kernel.elf build/mnt/kernel.elf
    umount build/mnt

    if [[ "$platform" == "Linux" ]]; then
        sudo losetup --detach $loopback
    elif [[ "$platform" == "Darwin" ]]; then
        hdiutil eject $device > /dev/null
    fi

    printf "[mkdisk] files copied to disk image.\n"
fi
