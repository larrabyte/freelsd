# ----------------------------------------------------
# Makefile for FreeLSD, made by the larrabyte himself.
# ----------------------------------------------------
.PHONY: all clean dump

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
LFLAGS    := -T kernel/src/$(ARCH)/linker.ld -z max-page-size=0x1000
ASFLAGS   := -felf64

all: build/freelsd.iso
-include $(KERNELDEP)

clean:
	@rm -f kernel/obj/*
	@rm -f build/isoroot/kernel.elf
	@rm -f build/freelsd.iso
	@rm -f scripts/disassembly.log
	@printf "[remove] removed build artefacts.\n"

dump: build/isoroot/kernel.elf
	@$(DUMPER) $(DUMPFLAGS) build/isoroot/kernel.elf > scripts/disassembly.log
	@printf "[dumper] kernel disassembly file created.\n"

build/freelsd.iso: build/isoroot/kernel.elf
	@grub-mkrescue -o build/freelsd.iso build/isoroot &> /dev/null
	@printf "[mkgrub] GRUB rescue ISO created.\n"

build/isoroot/kernel.elf: $(KERNELOBJ)
	@$(LINKER) $(LFLAGS) $(KERNELOBJ) -o build/isoroot/kernel.elf
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