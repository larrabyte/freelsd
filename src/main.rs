#![no_std]
#![no_main]

use core::panic::PanicInfo;
use core::arch::asm;

#[panic_handler]
fn panic(_: &PanicInfo) -> ! {
    unsafe {
        asm!("hlt", options(noreturn));
    }
}

#[no_mangle]
pub extern "C" fn main() -> ! {
    unsafe {
        asm!("hlt", options(noreturn));
    }
}
