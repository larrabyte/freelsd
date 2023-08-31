use super::InterruptStackFrame;

/// A handler for non-error code producing interrupts which immediately kernel panics.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn panic_without_error_code(_: InterruptStackFrame) {
    panic!("fatal processor exception");
}

/// A handler for error code producing interrupts which immediately kernel panics.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn panic_with_error_code(_: InterruptStackFrame, code: u64) {
    panic!("fatal processor exception with error code {}", code);
}
