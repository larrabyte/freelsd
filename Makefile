# ----------------------------------------------------
# Makefile for FreeLSD, made by the larrabyte himself.
# ----------------------------------------------------
.PHONY: default qemu qemudebug bochs bochsdebug tools clean cleanall

ARCH := x86_64
CPP  := x86_64-elf-g++
ASM  := nasm

# ----------------------------------
# Assember, compiler and QEMU flags.
# ----------------------------------
WARNINGS := -Wall -Wextra -Wpedantic -Wno-unused-parameter
CFLAGS   := $(WARNINGS) -ffreestanding -fstack-protector -fno-exceptions -fno-rtti \
			-mcmodel=kernel -mno-red-zone -mno-sse -zmax-page-size=0x1000 -O3 -nostdlib

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
default: qemu

qemu: build/freelsd.iso
	@printf "[qemuvm] Now booting FreeLSD (KVM/WHPX).\n"
	@./scripts/virtualise.sh qemu

qemudebug: build/freelsd.iso objdump
	@printf "[qemuvm] Now booting FreeLSD (QEMU debug).\n"
	@./scripts/virtualise.sh qemudebug

bochs: build/freelsd.iso
	@printf "[vbochs] Now booting FreeLSD (no acceleration).\n"
	@./scripts/virtualise.sh bochs

bochsdebug: build/freelsd.iso objdump
	@printf "[vbochs] Now booting FreeLSD (Bochs debug).\n"
	@./scripts/virtualise.sh bochsdebug

tools:
	@cd tools/initrdgen && $(MAKE) --no-print-directory

objdump:
	@printf "[objdmp] Creating new disassembly file for kernel.bin.\n"
	@objdump -D -M intel isoroot/kernel.bin > scripts/disassembly.log

clean:
	@rm -f $(KERNELOBJ)/*.o
	@printf "[remove] Deleted object files from kernel/obj.\n"
	@rm -f scripts/*.log
	@printf "[remove] Deleted log files from scripts/log.\n"

cleanall: clean
	@rm -f isoroot/kernel.bin
	@printf "[remove] Deleted isoroot/kernel.bin.\n"
	@rm -f build/freelsd.iso
	@printf "[remove] Deleted build/freelsd.iso.\n"
	@cd tools/initrdgen && $(MAKE) --no-print-directory clean

build/freelsd.iso: $(OBJFILES)
	@printf "[linker] Linking object files and creating ISO.\n"
	@$(CPP) -T $(KERNELSRC)/linker.ld $(CFLAGS) $(CRTBEGIN) $(sort $(OBJFILES)) $(CRTFINAL) -o isoroot/kernel.bin -lgcc
	@./tools/initrdgen/initrdgen
	@grub-mkrescue -o build/freelsd.iso isoroot &> /dev/null

$(KERNELOBJ)/%.o: $(KERNELSRC)/%.cpp
	@printf "[cppobj] $< compiled.\n"
	@$(CPP) $(CFLAGS) -I $(KERNELINC) -c $< -o $@

$(KERNELOBJ)/%.o: $(KERNELSRC)/%.asm
	@printf "[asmobj] $< assembled.\n"
	@$(ASM) $(AFLAGS) $< -o $@
