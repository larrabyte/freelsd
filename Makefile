# ----------------------------------------------------
# Makefile for FreeLSD, made by the larrabyte himself.
# ----------------------------------------------------
.PHONY: all i686 clean build/freelsd.iso

ARCH := i686
QEMU := qemu-system-i386
CPP  := $(ARCH)-elf-g++
ASM  := nasm

# ----------------------------------
# Assember, compiler and QEMU flags.
# ----------------------------------
WARNINGS := -Wall -Wextra -Wpedantic -Wno-unused-parameter
CFLAGS   := $(WARNINGS) -ffreestanding -fstack-protector \
			-fno-exceptions -fno-rtti -O3 -nostdlib

QFLAGS := -M q35 -display sdl -serial file:serial.log -cdrom build/freelsd.iso
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
OBJFILES := $(ASMFILES:$(KERNELSRC)/%.asm=$(KERNELOBJ)/%.o) $(CPPFILES:$(KERNELSRC)/%.cpp=$(KERNELOBJ)/%.o)

# --------
# Targets.
# --------
all: $(ARCH)

clean:
	@rm -f $(KERNELOBJ)/*.o
	@printf "[wipe] Deleted object files from kernel/obj.\n"
	@rm -f isoroot/kernel.bin
	@printf "[wipe] Deleted isoroot/kernel.bin.\n"
	@rm -f build/freelsd.iso
	@printf "[wipe] Deleted build/freelsd.iso.\n"
	@rm -f serial.log
	@printf "[wipe] Deleted serial.log.\n"

i686: build/freelsd.iso
	@printf "[qemu] Now booting FreeLSD.\n"
	@$(QEMU) $(QFLAGS)

build/freelsd.iso: $(OBJFILES)
	@printf "[link] Linking object files and creating ISO.\n"
	@$(CPP) -T $(KERNELSRC)/linker.ld $(CFLAGS) $(CRTBEGIN) $(sort $(OBJFILES)) $(CRTFINAL) -o isoroot/kernel.bin -lgcc
	@grub-mkrescue -o build/freelsd.iso isoroot &> /dev/null

$(KERNELOBJ)/%.o: $(KERNELSRC)/%.cpp
	@printf "[g+++] $< compiled.\n"
	@$(CPP) $(CFLAGS) -I $(KERNELINC) -c $< -o $@

$(KERNELOBJ)/%.o: $(KERNELSRC)/%.asm
	@printf "[nasm] $< assembled.\n"
	@$(ASM) $(AFLAGS) $< -o $@