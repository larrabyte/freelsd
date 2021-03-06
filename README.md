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
[See the wiki page on compiling FreeLSD.](https://github.com/larrabyte/freelsd/wiki/Compilation)

## Showcase (as of commit #176).
![FreeLSD Graphical Output and Serial Log](showcase.png)
