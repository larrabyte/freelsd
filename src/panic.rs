use crate::{serial, serialln, instructions, uart::COM1};
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

    serialln!();
    serialln!("a kernel panic has occurred!");

    serial!("location: ");
    match context.location() {
        Some(location) => serialln!("{}", location),
        None => serialln!("no location provided")
    };

    serial!("reason: ");
    match context.message() {
        Some(message) => serialln!("{}", message),
        None => serialln!("no reason provided")
    };

    loop {
        instructions::hlt();
    }
}
