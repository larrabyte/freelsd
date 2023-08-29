use crate::{serial, instructions, uart::COM1};
use core::panic::PanicInfo;

#[panic_handler]
fn panic(context: &PanicInfo) -> ! {
    // TODO: Halt all currently executing processors.
    instructions::cli();

    // Since all processors have stopped executing,
    // we could be in a situation where the serial
    // mutex is held but cannot be released.
    unsafe {
        COM1.force_unlock();
    }

    serial!("\n");
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
