# exec.sh
# made by the larrabyte himself
# cant make a makefile

EXEC="kernel"
ARCH="i386"

# compiler and assembler
CC="i686-elf-g++"
AS="nasm"

# flags
CFLAGS="-Wall -Wextra -Wno-unused-variable -ffreestanding -fstack-protector -fno-exceptions -fno-rtti -O2 -nostdlib"
IFLAGS="-Ikernel/include"
AFLAGS="-felf32"
LFLAGS="-lgcc"

# don't change these please :)
CRTBEGIN="$(${CC} ${CFLAGS} -print-file-name=crtbegin.o)"
CRTEND="$(${CC} ${CFLAGS} -print-file-name=crtend.o)"
OBJLIST=""
set -e

function kernelcompile() {
    for file in kernel/arch/${ARCH}/* ; do
        if [ -d "$file" ] ; then
            continue
        fi

        fbase=$(basename "$file")
        fname="${fbase%.*}"
        fext="${fbase##*.}"

        if [ "$fext" == "asm" ] ; then
            ${AS} ${AFLAGS} ${file} -o kernel/obj/${fname}.o
            printf "Assembling %s...\n" "$file"
        fi

        if [ "$fext" == "cpp" ] ; then
            ${CC} ${CFLAGS} ${IFLAGS} -c ${file} -o kernel/obj/${fname}.o
            printf "Compiling %s...\n" "$file"
        fi
    done
}

function kernellink() {
    for file in kernel/obj/* ; do
        OBJLIST="${OBJLIST}${file} "
    done

    printf "Linking object files...\n"
    ${CC} -T kernel/arch/${ARCH}/linker.ld ${CFLAGS} ${CRTBEGIN} ${OBJLIST} ${CRTEND} -o isoroot/${EXEC}.bin ${LFLAGS}
}

function kerneliso() {
    printf "Creating a bootable ISO... (grub.cfg)\n"
    echo -ne "menuentry \"FreeLSD\" {\n    multiboot /${EXEC}.bin\n}" > isoroot/boot/grub/grub.cfg
    printf "Creating a bootable ISO... (grub-mkrescue)\n"
    grub-mkrescue -o freelsd.iso isoroot &> /dev/null
}

# argument parser
if [ "$1" == "build" ] ; then
    kernelcompile
    kernellink
    kerneliso
fi

qemu-system-i386 -M q35 -display sdl -cdrom freelsd.iso