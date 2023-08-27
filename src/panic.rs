use crate::{serial, instructions};

use core::panic::PanicInfo;

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

    loop {
        instructions::hlt();
    }
}
