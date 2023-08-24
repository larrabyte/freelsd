use core::arch::global_asm;

extern "sysv64" fn nop() {}

global_asm!(
    ".globl interrupt_handler_without_error_code",
    "interrupt_handler_without_error_code:",
    "push rax",
    "push rdi",
    "push rsi",
    "push rdx",
    "push rcx",
    "push r8",
    "push r9",
    "push r10",
    "push r11",
    "sub rsp, 8", // 16-byte stack alignment.
    "call {}",
    "add rsp, 8",
    "pop r11",
    "pop r10",
    "pop r9",
    "pop r8",
    "pop rcx",
    "pop rdx",
    "pop rsi",
    "pop rdi",
    "pop rax",
    "iretq",
    sym nop
);

global_asm!(
    ".globl interrupt_handler_with_error_code",
    "interrupt_handler_with_error_code:",
    "push rax",
    "push rdi",
    "push rsi",
    "push rdx",
    "push rcx",
    "push r8",
    "push r9",
    "push r10",
    "push r11",
    "call {}",
    "pop r11",
    "pop r10",
    "pop r9",
    "pop r8",
    "pop rcx",
    "pop rdx",
    "pop rsi",
    "pop rdi",
    "pop rax",
    "add rsp, 8", // Deallocate the error code.
    "iretq",
    sym nop
);
