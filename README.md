# freelsd
* A personal software project designed to run on x86_64 systems.
* This is just for fun, so don't expect anything usable for a while.
* See [Lemon](https://github.com/fido2020/lemon-os) for reasons I shouldn't be doing this.

## Features
FreeLSD currently has the following features implemented in the 64-bit branch:
* Support for Multiboot2, SSP and VESA graphics.
* Virtual and physical memory management.
* APIC-driven or PIC-driven interrupts.
* Support for SMP using ACPI <1.0.
* And of course, the FreeLSD frog!

## Minimum Requirements
Tested on QEMU, VMWare, Bochs and on bare-metal using my laptop. VirtualBox doesn't support huge pages when booting x86_64 systems unfortunately.
| x86_64                             |
| ---------------------------------- |
| 8MB of physical memory.            |
| Single-core x86_64 processor.      |
| Multiboot2 bootloader.             |
| Support for huge pages (PDPE1GB).  |
| Support for ACPI.                  |

## Compilation
FreeLSD uses Clang/LLVM (and NASM on x86_64) for compiling and assembling. Clang/LLVM is used here because it is inherently a cross compiler and thus does not require a recompilation to target new architectures.

Simply install prerequisites via your favourite package manager.
```
$ sudo pacman -S llvm lld clang nasm (on Arch Linux)
$ brew install llvm nasm (on macOS)
```

Once the compiler and assembler are installed, execute `sudo make disk` to create an empty disk with the correct partition layout for FreeLSD.
> I couldn't get createdisk.sh to work on macOS, so if you're trying to build FreeLSD on macOS then you'll have to run this script on an external machine and copy the disk image over to your own machine.

Now that everything is ready, invoke the Makefile using `sudo make`. This will compile and link the kernel before copying it to the disk image and running QEMU. Running Bochs can also be done using `sudo make bochs`.
> The Makefile requires root on Linux because of losetup. If you're building on macOS, sudo is not required.

## Showcase (as of commit #176).
![FreeLSD Graphical Output and Serial Log](showcase.png)
