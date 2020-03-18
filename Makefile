# ----------------------------------------------------
# Makefile for FreeLSD, made by the larrabyte himself.
# ----------------------------------------------------
.PHONY: all i686 clean

ARCH := i686
QEMU := qemu-system-i386
CPP  := $(ARCH)-elf-g++
ASM  := nasm

# ----------------------------------
# Assember, compiler and QEMU flags.
# ----------------------------------
WARNINGS := -Wall -Wextra -Wpedantic -Wno-unused-parameter
CFLAGS   := $(WARNINGS) -ffreestanding -fstack-protector \
			-fno-exceptions -fno-rtti -O2 -nostdlib

QFLAGS := -M q35 -display sdl -cdrom build/freelsd.iso
AFLAGS := -felf32

# -----------------------------
# Required directories & files.
# -----------------------------
KERNELSRC := kernel/arch/$(ARCH)
KERNELINC := kernel/include
KERNELOBJ := kernel/obj

CPPFILES := $(wildcard $(KERNELSRC)/*.cpp)
ASMFILES := $(wildcard $(KERNELSRC)/*.asm)
CRTFINAL := $(shell $(CPP) $(CFLAGS) -print-file-name=crtend.o)
CRTBEGIN := $(shell $(CPP) $(CFLAGS) -print-file-name=crtbegin.o)
OBJFILES := $(sort $(ASMFILES:$(KERNELSRC)/%.asm=$(KERNELOBJ)/%.o) $(CPPFILES:$(KERNELSRC)/%.cpp=$(KERNELOBJ)/%.o))

# --------
# Targets.
# --------
all: $(ARCH)

clean:
	rm -f build/freelsd.iso
	rm -f isoroot/kernel.bin
	rm -f $(KERNELOBJ)/*.o

i686: build/freelsd.iso
	$(QEMU) $(QFLAGS)

build/freelsd.iso: $(OBJFILES)
	$(CPP) -T $(KERNELSRC)/linker.ld $(CFLAGS) $(CRTBEGIN) $(OBJFILES) $(CRTFINAL) -o isoroot/kernel.bin -lgcc
	grub-mkrescue -o build/freelsd.iso isoroot

$(KERNELOBJ)/%.o: $(KERNELSRC)/%.cpp
	$(CPP) $(CFLAGS) -I $(KERNELINC) -c $< -o $@

$(KERNELOBJ)/%.o: $(KERNELSRC)/%.asm
	$(ASM) $(AFLAGS) $< -o $@