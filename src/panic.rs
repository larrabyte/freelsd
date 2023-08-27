use crate::serial;

use core::{arch::asm, panic::PanicInfo};

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
