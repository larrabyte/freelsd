# ----------------------------------------------------
# Makefile for FreeLSD, made by the larrabyte himself.
# ----------------------------------------------------
.PHONY: all clean dump

TARGET  := x86_64-elf
ARCH    := x86_64

CPP     := clang++
ASM     := nasm
LINKER  := ld.lld
DUMPER  := llvm-objdump

# -----------------------------
# Required directories & files.
# -----------------------------
KERNELSRC := $(shell find kernel/src -maxdepth 1 -name "*.cpp")
KERNELSRC += $(shell find kernel/src/arch/$(ARCH) -maxdepth 1 -name "*.cpp" -o -name "*.asm")
KERNELOBJ := $(addprefix kernel/obj/, $(notdir $(addsuffix .o, $(basename $(KERNELSRC)))))

# -----------------------------------------------------------------
# Assember, compiler, linker and disassembler flags for the kernel.
# -----------------------------------------------------------------
WARNINGS  := -Wall -Wextra -Wpedantic -Wno-builtin-macro-redefined
REDEFINES := -D__TIMESTAMP__=\"$(shell date +'"%A, %d %B %Y %r %Z"')\"
CFLAGS    := $(WARNINGS) $(REDEFINES) --target=$(TARGET) -Ikernel/include -Ikernel/include/$(ARCH) \
			 --std=c++20 -ffreestanding -fstack-protector -fno-exceptions -fno-rtti \
			 -mcmodel=kernel -mno-red-zone -mno-sse -O2 -nostdlib

DUMPFLAGS := --arch=$(TARGET) --disassemble --demangle --print-imm-hex --x86-asm-syntax=intel
LFLAGS    := -T kernel/src/arch/$(ARCH)/linker.ld -z max-page-size=0x1000
AFLAGS    := -felf64

all: build/freelsd.iso

clean:
	@rm --force kernel/obj/*.o
	@rm --force build/isoroot/kernel.bin
	@rm --force build/freelsd.iso
	@rm --force scripts/disassembly.log
	@printf "[remove] removed build artefacts.\n"

dump: build/isoroot/kernel.bin
	@$(DUMPER) $(DUMPFLAGS) build/isoroot/kernel.bin > scripts/disassembly.log
	@printf "[dumper] kernel disassembly file created.\n"

build/freelsd.iso: build/isoroot/kernel.bin
	@grub-mkrescue -o build/freelsd.iso build/isoroot &> /dev/null
	@printf "[mkgrub] GRUB rescue ISO created.\n"

build/isoroot/kernel.bin: $(KERNELOBJ)
	@$(LINKER) $(LFLAGS) $(KERNELOBJ) -o build/isoroot/kernel.bin
	@printf "[linker] %s files successfully linked.\n" $(words $(KERNELOBJ))

kernel/obj/%.o: kernel/src/%.cpp
	@$(CPP) $(CFLAGS) -c $< -o $@
	@printf "[cppobj] $< compiled.\n"

kernel/obj/%.o: kernel/src/arch/$(ARCH)/%.cpp
	@$(CPP) $(CFLAGS) -c $< -o $@
	@printf "[cppobj] $< compiled.\n"

kernel/obj/%.o: kernel/src/arch/$(ARCH)/%.asm
	@$(ASM) $(AFLAGS) $< -o $@
	@printf "[asmobj] $< assembled.\n"