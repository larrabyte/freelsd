# ----------------------------------------------------
# Makefile for FreeLSD, made by the larrabyte himself.
# ----------------------------------------------------
.PHONY: default tools clean cleanall

ARCH := x86_64
QEMU := qemu-system-x86_64
CPP  := /usr/x86_64-elf/bin/x86_64-elf-g++
ASM  := nasm

# ----------------------------------
# Assember, compiler and QEMU flags.
# ----------------------------------
WARNINGS := -Wall -Wextra -Wpedantic -Wno-unused-parameter
CFLAGS   := $(WARNINGS) -ffreestanding -fstack-protector -fno-exceptions -fno-rtti \
			-mcmodel=kernel -mno-red-zone -zmax-page-size=0x1000 -O2 -nostdlib

QFLAGS := -no-reboot -no-shutdown -serial stdio -display sdl -M q35,accel=whpx:kvm:tcg -cdrom build/freelsd.iso
AFLAGS := -felf64

# -----------------------------
# Required directories & files.
# -----------------------------
KERNELSRC := kernel/arch/$(ARCH)
KERNELINC := kernel/include/$(ARCH)
KERNELOBJ := kernel/obj

CPPFILES := $(wildcard $(KERNELSRC)/*.cpp)
ASMFILES := $(wildcard $(KERNELSRC)/*.asm)
CRTFINAL := $(shell $(CPP) $(CFLAGS) -print-file-name=crtend.o)
CRTBEGIN := $(shell $(CPP) $(CFLAGS) -print-file-name=crtbegin.o)
OBJFILES := $(ASMFILES:$(KERNELSRC)/%.asm=$(KERNELOBJ)/%.o) $(CPPFILES:$(KERNELSRC)/%.cpp=$(KERNELOBJ)/%.o)

# --------
# Targets.
# --------
default: build/freelsd.iso
	@printf "[qmu] Now booting FreeLSD.\n"
	@$(QEMU) $(QFLAGS)

tools:
	@cd tools/initrdgen && $(MAKE) --no-print-directory

clean:
	@rm -f $(KERNELOBJ)/*.o
	@printf "[del] Deleted object files from kernel/obj.\n"

cleanall: clean
	@rm -f isoroot/kernel.bin
	@printf "[del] Deleted isoroot/kernel.bin.\n"
	@rm -f build/freelsd.iso
	@printf "[del] Deleted build/freelsd.iso.\n"
	@cd tools/initrdgen && $(MAKE) --no-print-directory clean

build/freelsd.iso: $(OBJFILES)
	@printf "[g++] Linking object files and creating ISO.\n"
	@$(CPP) -T $(KERNELSRC)/linker.ld $(CFLAGS) $(CRTBEGIN) $(sort $(OBJFILES)) $(CRTFINAL) -o isoroot/kernel.bin -lgcc
	@./tools/initrdgen/initrdgen
	@grub-mkrescue -o build/freelsd.iso isoroot &> /dev/null

$(KERNELOBJ)/%.o: $(KERNELSRC)/%.cpp
	@printf "[g++] $< compiled.\n"
	@$(CPP) $(CFLAGS) -I $(KERNELINC) -c $< -o $@

$(KERNELOBJ)/%.o: $(KERNELSRC)/%.asm
	@printf "[asm] $< assembled.\n"
	@$(ASM) $(AFLAGS) $< -o $@
