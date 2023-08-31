use super::InterruptStackFrame;

/// Handles processor division by zero exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn division_by_zero(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: division by zero\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor debug exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn debug(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: debug\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor NMIs.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn non_maskable_interrupt(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: non-maskable interrupt\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor breakpoint exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn breakpoint(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: breakpoint\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor overflow exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn overflow(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: overflow\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor bounds range exceeded exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn bound_range_exceeded(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: bound range exceeded\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor invalid opcode exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn invalid_opcode(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: invalid opcode\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor device not available exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn device_not_available(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: device not available\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor double fault exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn double_fault(context: InterruptStackFrame, code: u64) {
    panic!(
        concat!(
            "fatal processor exception: double fault\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}\n",
            "      code: {:#x}",
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags,
        code
    );
}

/// Handles processor coprocessor segment overrun exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn coprocessor_segment_overrun(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: coprocessor segment overrun\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor invalid task state segment exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn invalid_task_state_segment(context: InterruptStackFrame, code: u64) {
    panic!(
        concat!(
            "fatal processor exception: invalid task state segment\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}\n",
            "      code: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags,
        code
    );
}

/// Handles processor segment not present exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn segment_not_present(context: InterruptStackFrame, code: u64) {
    panic!(
        concat!(
            "fatal processor exception: segment not present\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}\n",
            "      code: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags,
        code
    );
}

/// Handles processor stack segment faults.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn stack_segment_fault(context: InterruptStackFrame, code: u64) {
    panic!(
        concat!(
            "fatal processor exception: stack segment fault\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}\n",
            "      code: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags,
        code
    );
}

/// Handles processor general protection faults.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn general_protection_fault(context: InterruptStackFrame, code: u64) {
    panic!(
        concat!(
            "fatal processor exception: general protection fault\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}\n",
            "      code: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags,
        code
    );
}

/// Handles processor page faults.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn page_fault(context: InterruptStackFrame, code: u64) {
    panic!(
        concat!(
            "fatal processor exception: page fault\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}\n",
            "      code: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags,
        code
    );
}

/// Handles processor x87 floating-point exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn x87_floating_point_exception(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: x87 floating-point\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}",
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor alignment check exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn alignment_check(context: InterruptStackFrame, code: u64) {
    panic!(
        concat!(
            "fatal processor exception: alignment check\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}\n",
            "      code: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags,
        code
    );
}

/// Handles processor machine check exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn machine_check(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: machine check\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor SIMD floating-point exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn simd_floating_point_exception(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: SIMD floating-point\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor virtualisation exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn virtualisation_exception(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: virtualisation\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor control protection exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn control_protection_exception(context: InterruptStackFrame, code: u64) {
    panic!(
        concat!(
            "fatal processor exception: control protection\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}\n",
            "      code: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags,
        code
    );
}

/// Handles processor hypervisor injection exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn hypervisor_injection_exception(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception: alignment check\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}

/// Handles processor VMM communication exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn vmm_communication_exception(context: InterruptStackFrame, code: u64) {
    panic!(
        concat!(
            "fatal processor exception: VMM communication\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}\n",
            "      code: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags,
        code
    );
}

/// Handles processor security exceptions.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn security_exception(context: InterruptStackFrame, code: u64) {
    panic!(
        concat!(
            "fatal processor exception: security\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}\n",
            "      code: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags,
        code
    );
}

/// A handler for non-error code producing interrupts which immediately kernel panics.
///
/// # Safety
/// The only sanctioned method of invoking this handler is via the processor interrupt mechanism.
pub unsafe extern "x86-interrupt" fn panic_without_error_code(context: InterruptStackFrame) {
    panic!(
        concat!(
            "fatal processor exception\n",
            "context:\n",
            "    cs:rip: {:#04x}:{:#x}\n",
            "    ss:rsp: {:#04x}:{:#x}\n",
            "    rflags: {:#x}"
        ),
        context.cs,
        context.rip,
        context.ss,
        context.rsp,
        context.rflags
    );
}
