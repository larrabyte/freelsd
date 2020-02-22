# exec.sh
# made by the larrabyte himself
# cant make a makefile

EXEC="kernel"

# compiler and assembler (for boot.asm)
CC="i686-elf-gcc"
AS="nasm"

# flags
CFLAGS="-Wall -Wextra -O2 -ffreestanding -std=gnu99"
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

        if [ "$fext" == "c" ] ; then
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

# argument parser
if [ "$1" == "build" ] ; then
    stageone
    stagetwo
fi

qemu-system-i386 -display sdl -kernel bin/kernel.bin