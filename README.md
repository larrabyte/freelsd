# freelsd
An attempt to make an OS, following the [OSDev Wiki](https://wiki.osdev.org) along with what I presume will be many days of fustration. See [Lemon](https://github.com/fido2020/Lemon-OS) and [OSDev: Beginner Mistakes](https://wiki.osdev.org/Beginner_Mistakes) for reasons I shouldn't be doing this.


## Features
FreeLSD currently has the following features:
* Interrupt handling (currently implemented in the keyboard/timer).
* Compatiability with multiboot bootloaders and VESA graphics.
* Protection against stack smashing (via the SSP).
* Basic support for writing to serial.
* The FreeLSD frog!


## Prerequisites and Compilation
You'll need `nasm` and `i686-elf-gcc` to compile FreeLSD. I use Arch Linux, so for fellow users, simply type the following in a terminal:
```
sudo pacman -S nasm
```
```
yay -S i686-elf-gcc
```
NOTE: `i686-elf-gcc` cannot be found using `pacman`, it is hosted on the Arch User Repository. I use [yay](https://github.com/Jguer/yay) to manage my installed AUR packages, it won't matter what manager you use as long as you have `i686-elf-gcc` installed.

If you simply want to run FreeLSD in a VM, install `QEMU` via the following command (the Makefile also immediately boots QEMU for you):
```
sudo pacman -S qemu qemu-arch-extra
```

And once you've got the tools required to compile FreeLSD:
```
git clone https://github.com/larrabyte/freelsd
cd freelsd
make
```
Boom! FreeLSD compiled and running on your system.


## Showcase (as of commit #78, just after [this](https://github.com/larrabyte/freelsd/commit/6a935549e634e4014af481627598e9fe316f96f8) one.)
![FreeLSD Graphical Output and Serial Log](showcase.png)
