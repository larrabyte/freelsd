#![feature(panic_info_message)]
#![feature(abi_x86_interrupt)]
#![no_main]
#![no_std]

mod instructions;
mod ports;
mod panic;
mod boot;
mod uart;
mod idt;
mod gdt;

use crate::boot::BOOTLOADER_INFORMATION;

#[no_mangle]
extern "C" fn main() -> ! {
    serialln!("\n  ()-()\n.-(___)-. freelsd kernel\n _<   >_  you know the last plane out of sydney's almost gone?\n \\/   \\/\n");

    let bootloader = BOOTLOADER_INFORMATION.get_response().get().unwrap();
    serialln!("bootloader name: {:?}", bootloader.name.to_str().unwrap());
    serialln!("bootloader version: {:?}", bootloader.version.to_str().unwrap());

    gdt::load();
    idt::load();

    loop {
        instructions::hlt();
    }
}
