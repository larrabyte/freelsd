use core::{arch::asm, marker::PhantomData, mem::size_of};

#[derive(Debug, Clone, Copy)]
#[repr(transparent)]
struct InterruptGate<E>(u128, PhantomData<E>);

#[derive(Debug, Clone, Copy)]
struct WithErrorCode;

#[derive(Debug, Clone, Copy)]
struct WithoutErrorCode;

#[derive(Debug, Default)]
#[repr(C)]
pub struct InterruptDescriptorTable {
    division_by_zero: InterruptGate<WithoutErrorCode>,
    debug: InterruptGate<WithoutErrorCode>,
    non_maskable_interrupt: InterruptGate<WithoutErrorCode>,
    breakpoint: InterruptGate<WithoutErrorCode>,
    overflow: InterruptGate<WithoutErrorCode>,
    bound_range_exceeded: InterruptGate<WithoutErrorCode>,
    invalid_opcode: InterruptGate<WithoutErrorCode>,
    device_not_available: InterruptGate<WithoutErrorCode>,
    double_fault: InterruptGate<WithErrorCode>,
    coprocessor_segment_overrun: InterruptGate<WithoutErrorCode>,
    invalid_task_state_segment: InterruptGate<WithErrorCode>,
    segment_not_present: InterruptGate<WithErrorCode>,
    stack_segment_fault: InterruptGate<WithErrorCode>,
    general_protection_fault: InterruptGate<WithErrorCode>,
    page_fault: InterruptGate<WithErrorCode>,
    reserved_15: InterruptGate<WithoutErrorCode>,
    x87_floating_point_exception: InterruptGate<WithoutErrorCode>,
    alignment_check: InterruptGate<WithErrorCode>,
    machine_check: InterruptGate<WithoutErrorCode>,
    simd_floating_point_exception: InterruptGate<WithoutErrorCode>,
    virtualisation_exception: InterruptGate<WithoutErrorCode>,
    control_protection_exception: InterruptGate<WithErrorCode>,
    reserved_22_to_27: [InterruptGate<WithoutErrorCode>; 6],
    hypervisor_injection_exception: InterruptGate<WithoutErrorCode>,
    vmm_communication_exception: InterruptGate<WithErrorCode>,
    security_exception: InterruptGate<WithErrorCode>,
    reserved_31: InterruptGate<WithoutErrorCode>
}

#[derive(Debug)]
#[repr(C, packed)]
struct IdtPointer {
    size: u16,
    pointer: *const InterruptDescriptorTable
}

#[derive(Debug)]
#[repr(C)]
pub struct InterruptStackFrame {
    rip: u64,
    cs: u64,
    rflags: u64,
    rsp: u64,
    ss: u64
}

extern "sysv64" {
    fn interrupt_handler_without_error_code();
    fn interrupt_handler_with_error_code();
}

impl Default for InterruptGate<WithoutErrorCode> {
    fn default() -> Self {
        let handler = interrupt_handler_without_error_code as usize as u128;
        let target = handler & 0xFFFF | ((handler & 0xFFFF0000) << 32) | ((handler & 0xFFFFFFFF00000000) << 32);
        let segment = 1 << 19;

        // Target offset + kernel code segment + 64-bit interrupt gate + present.
        Self(target | segment | 0b1110 << 40 | 1 << 47, PhantomData)
    }
}

impl Default for InterruptGate<WithErrorCode> {
    fn default() -> Self {
        let handler = interrupt_handler_with_error_code as usize as u128;
        let target = handler & 0xFFFF | ((handler & 0xFFFF0000) << 32) | ((handler & 0xFFFFFFFF00000000) << 32);
        let segment = 1 << 19;

        // Target offset + kernel code segment + 64-bit interrupt gate + present.
        Self(target | segment | 0b1110 << 40 | 1 << 47, PhantomData)
    }
}

impl InterruptDescriptorTable {
    /// Loads the IDTR with this interrupt descriptor table.
    ///
    /// # Safety
    /// The IDTR should only be loaded during kernel initialisation.
    pub unsafe fn load(&'static self) {
        let ptr = IdtPointer {
            size: (size_of::<InterruptDescriptorTable>() - 1) as u16,
            pointer: self as *const InterruptDescriptorTable
        };

        asm!("lidt [{}]", in(reg) &ptr, options(preserves_flags));
    }
}
