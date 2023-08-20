#![feature(panic_info_message)]
#![no_main]
#![no_std]

mod ports;
mod boot;
mod uart;
mod gdt;

use crate::boot::BOOTLOADER_INFORMATION;

use core::{panic::PanicInfo, arch::asm};

#[panic_handler]
fn panic(context: &PanicInfo) -> ! {
    serial!("Kernel panic: ");

    match context.message() {
        Some(message) => serial!("{}", message),
        None => serial!("(no message provided)")
    };

    serial!(", ");

    match context.location() {
        Some(location) => serial!("{}", location),
        None => serial!("(no message provided)")
    };

    serial!("\n");

    unsafe {
        asm!("hlt", options(noreturn));
    }
}

#[no_mangle]
pub extern "C" fn main() -> ! {
    serialln!("\n  ()-()\n.-(___)-. freelsd kernel\n _<   >_  you know the last plane out of sydney's almost gone?\n \\/   \\/\n");

    let bootloader = BOOTLOADER_INFORMATION.get_response().get().unwrap();
    serialln!("Bootloader name: {:?}", bootloader.name.to_str().unwrap());
    serialln!("Bootloader version: {:?}", bootloader.version.to_str().unwrap());

    unsafe {
        gdt::load(&mut gdt::GDT);
        asm!("hlt", options(noreturn));
    }
}
