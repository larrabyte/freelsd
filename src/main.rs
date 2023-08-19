#![no_main]
#![no_std]

mod ports;
mod boot;
mod uart;

use crate::boot::BOOTLOADER_INFORMATION;

use core::{panic::PanicInfo, arch::asm};

#[panic_handler]
fn panic(_: &PanicInfo) -> ! {
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
        asm!("hlt", options(noreturn));
    }
}
