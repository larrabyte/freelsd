#![no_std]
#![no_main]

mod ports;
mod uart;

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

    unsafe {
        asm!("hlt", options(noreturn));
    }
}
