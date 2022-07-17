# ----------------------------------------------------
# Makefile for FreeLSD, made by the larrabyte himself.
# ----------------------------------------------------
.PHONY: all bochs qemu clean build dump

TARGET  := x86_64-elf
ARCH    := x86_64

CXX     := clang++
AS      := nasm
LINKER  := ld.lld
DUMPER  := llvm-objdump

# -----------------------------
# Required directories & files.
# -----------------------------
KERNELSRC := $(shell find kernel -maxdepth 1 -name "*.cpp")
KERNELSRC += $(shell find kernel/src/$(ARCH) -maxdepth 1 -name "*.cpp" -o -name "*.asm")
KERNELOBJ := $(addprefix kernel/obj/, $(notdir $(addsuffix .o, $(basename $(KERNELSRC)))))
KERNELDEP := $(addprefix kernel/obj/, $(notdir $(patsubst %.cpp, %.d, $(filter %.cpp, $(KERNELSRC)))))

# -----------------------------------------------------------------
# Assember, compiler, linker and disassembler flags for the kernel.
# -----------------------------------------------------------------
WARNINGS  := -Wall -Wextra -Wpedantic -Wno-builtin-macro-redefined
REDEFINES := -D__TIMESTAMP__=\"$(shell date +'"%A, %d %B %Y %r %Z"')\"
CXXFLAGS  := $(WARNINGS) $(REDEFINES) --target=$(TARGET) -Ikernel/include -Ikernel/include/$(ARCH) \
			 --std=c++20 -ffreestanding -fstack-protector -fno-exceptions -fno-rtti \
			 -mcmodel=kernel -mno-red-zone -mno-sse -nostdlib -MD -MP -O2

DUMPFLAGS := --arch=$(TARGET) --disassemble --demangle --print-imm-hex --x86-asm-syntax=intel
LFLAGS    := -T kernel/src/$(ARCH)/linker.ld
ASFLAGS   := -felf64

all: install
-include $(KERNELDEP)

qemu: install
	@qemu-system-x86_64 \
	-no-reboot \
	-no-shutdown \
	-serial stdio \
	-drive format=raw,file=build/disk.img \
	-M q35,accel=kvm \
	-cpu host \
	-smp 4

bochs: install
	@bochs -q -f scripts/bochsrc.bxrc

dump: build
	@$(DUMPER) $(DUMPFLAGS) build/kernel.elf > scripts/disassembly.log
	@printf "[dumper] kernel disassembly file created.\n"

clean:
	@rm -f kernel/obj/*
	@rm -f build/kernel.elf
	@printf "[remove] removed build artefacts.\n"

disk:
	@./scripts/createdisk.sh

install: build
	@./scripts/mountdisk.sh install

build: \
	build/kernel.elf \
	build/grub.cfg

build/kernel.elf: $(KERNELOBJ)
	@$(LINKER) $(LFLAGS) $(KERNELOBJ) -o build/kernel.elf
	@printf "[linker] %s files successfully linked.\n" $(words $(KERNELOBJ))

kernel/obj/%.o: kernel/%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@printf "[cxxobj] $< compiled.\n"

kernel/obj/%.o: kernel/src/$(ARCH)/%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@printf "[cxxobj] $< compiled.\n"

kernel/obj/%.o: kernel/src/$(ARCH)/%.asm
	@$(AS) $(ASFLAGS) $< -o $@
	@printf "[asmobj] $< assembled.\n"
