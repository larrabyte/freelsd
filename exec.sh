# exec.sh
# made by the larrabyte himself
# cant make a makefile

EXEC="kernel"

# compiler and assembler (for boot.asm)
CC="i686-elf-g++"
AS="nasm"

# flags
CFLAGS="-Wall -Wextra -Wno-unused-variable -ffreestanding -fno-exceptions -fno-rtti -O2"
FFLAGS="-nostdlib"
AFLAGS="-felf32"
LFLAGS="-lgcc"

# do not change these :)
OBJLIST=""
set -e

function stageone() {
    for file in src/* ; do
        if [ -d "$file" ] ; then
            continue
        fi

        fbase=$(basename "$file")
        fname="${fbase%.*}"
        fext="${fbase##*.}"

        if [ "$fext" == "asm" ] ; then
            ${AS} ${AFLAGS} ${file} -o obj/${fname}.o
            printf "Assembling %s...\n" "$file"
        fi

        if [ "$fext" == "cpp" ] ; then
            ${CC} ${CFLAGS} -c ${file} -o obj/${fname}.o
            printf "Compiling %s...\n" "$file"
        fi
    done
}

function stagetwo() {
    for file in obj/* ; do
        OBJLIST="${OBJLIST}${file} "
    done

    printf "Linking object files...\n"
    ${CC} -T src/linker.ld ${CFLAGS} ${FFLAGS} ${OBJLIST} -o bin/${EXEC}.bin ${LFLAGS}
}

function stagethree() {
    printf "Creating a bootable ISO... (grub.cfg)\n"
    echo -ne "menuentry \"FreeLSD\" {\n    multiboot /${EXEC}.bin\n}" > bin/boot/grub/grub.cfg
    printf "Creating a bootable ISO... (grub-mkrescue)\n"
    grub-mkrescue -o iso/freelsd.iso bin &> /dev/null
}

# argument parser
if [ "$1" == "build" ] ; then
    stageone
    stagetwo
    stagethree
fi

qemu-system-i386 -M q35 -display sdl -cdrom iso/freelsd.iso