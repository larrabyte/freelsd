use self::handlers::{
    division_by_zero,
    debug,
    non_maskable_interrupt,
    breakpoint,
    overflow,
    bound_range_exceeded,
    invalid_opcode,
    device_not_available,
    double_fault,
    coprocessor_segment_overrun,
    invalid_task_state_segment,
    segment_not_present,
    stack_segment_fault,
    general_protection_fault,
    page_fault,
    x87_floating_point_exception,
    alignment_check,
    machine_check,
    simd_floating_point_exception,
    virtualisation_exception,
    control_protection_exception,
    hypervisor_injection_exception,
    vmm_communication_exception,
    security_exception,
    panic_without_error_code
};

use core::{arch::asm, mem::size_of, marker::PhantomData};
use spin::Lazy;

mod handlers;

#[derive(Debug)]
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
    reserved_31_to_255: [InterruptGate<WithoutErrorCode>; 225]
}

#[derive(Debug, Clone, Copy)]
#[repr(transparent)]
struct InterruptGate<E>(u128, PhantomData<E>);

#[derive(Debug, Clone, Copy)]
struct WithErrorCode;

#[derive(Debug, Clone, Copy)]
struct WithoutErrorCode;

#[derive(Debug)]
#[repr(C, packed)]
struct IdtPointer {
    size: u16,
    ptr: *const InterruptDescriptorTable
}

#[derive(Debug)]
#[repr(C)]
pub struct InterruptStackFrame {
    pub rip: u64,
    pub cs: u64,
    pub rflags: u64,
    pub rsp: u64,
    pub ss: u64
}

static IDT: Lazy<InterruptDescriptorTable> = Lazy::new(InterruptDescriptorTable::new);

/// Loads the IDTR with the kernel's interrupt descriptor table.
pub fn load() {
    unsafe {
        IDT.load();
    }
}

impl InterruptDescriptorTable {
    /// Creates a pre-populated interrupt descriptor table.
    fn new() -> Self {
        Self {
            division_by_zero: InterruptGate::<WithoutErrorCode>::new(division_by_zero),
            debug: InterruptGate::<WithoutErrorCode>::new(debug),
            non_maskable_interrupt: InterruptGate::<WithoutErrorCode>::new(non_maskable_interrupt),
            breakpoint: InterruptGate::<WithoutErrorCode>::new(breakpoint),
            overflow: InterruptGate::<WithoutErrorCode>::new(overflow),
            bound_range_exceeded: InterruptGate::<WithoutErrorCode>::new(bound_range_exceeded),
            invalid_opcode: InterruptGate::<WithoutErrorCode>::new(invalid_opcode),
            device_not_available: InterruptGate::<WithoutErrorCode>::new(device_not_available),
            double_fault: InterruptGate::<WithErrorCode>::new(double_fault),
            coprocessor_segment_overrun: InterruptGate::<WithoutErrorCode>::new(coprocessor_segment_overrun),
            invalid_task_state_segment: InterruptGate::<WithErrorCode>::new(invalid_task_state_segment),
            segment_not_present: InterruptGate::<WithErrorCode>::new(segment_not_present),
            stack_segment_fault: InterruptGate::<WithErrorCode>::new(stack_segment_fault),
            general_protection_fault: InterruptGate::<WithErrorCode>::new(general_protection_fault),
            page_fault: InterruptGate::<WithErrorCode>::new(page_fault),
            reserved_15: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            x87_floating_point_exception: InterruptGate::<WithoutErrorCode>::new(x87_floating_point_exception),
            alignment_check: InterruptGate::<WithErrorCode>::new(alignment_check),
            machine_check: InterruptGate::<WithoutErrorCode>::new(machine_check),
            simd_floating_point_exception: InterruptGate::<WithoutErrorCode>::new(simd_floating_point_exception),
            virtualisation_exception: InterruptGate::<WithoutErrorCode>::new(virtualisation_exception),
            control_protection_exception: InterruptGate::<WithErrorCode>::new(control_protection_exception),
            reserved_22_to_27: [InterruptGate::<WithoutErrorCode>::new(panic_without_error_code); 6],
            hypervisor_injection_exception: InterruptGate::<WithoutErrorCode>::new(hypervisor_injection_exception),
            vmm_communication_exception: InterruptGate::<WithErrorCode>::new(vmm_communication_exception),
            security_exception: InterruptGate::<WithErrorCode>::new(security_exception),
            reserved_31_to_255: [InterruptGate::<WithoutErrorCode>::new(panic_without_error_code); 225]
        }
    }

    /// Loads the IDTR with this interrupt descriptor table.
    ///
    /// # Safety
    /// Callers must ensure that all interrupt gates are present, marked as 64-bit and set to DPL 0.
    unsafe fn load(&'static self) {
        let ptr = IdtPointer {
            size: (size_of::<InterruptDescriptorTable>() - 1) as u16,
            ptr: self as *const InterruptDescriptorTable
        };

        asm!("lidt [{}]", in(reg) &ptr, options(preserves_flags));
    }
}

impl InterruptGate<WithoutErrorCode> {
    fn new(f: unsafe extern "x86-interrupt" fn(InterruptStackFrame)) -> Self {
        let address = f as u128;
        let target = address & 0xffff | ((address & 0xffff_0000) << 32) | ((address & 0xffff_ffff_0000_0000) << 32);
        let segment = 1 << 19;

        // Target offset + kernel code segment + 64-bit interrupt gate + present.
        Self(target | segment | 0b1110 << 40 | 1 << 47, PhantomData)
    }
}

impl InterruptGate<WithErrorCode> {
    fn new(f: unsafe extern "x86-interrupt" fn(InterruptStackFrame, u64)) -> Self {
        let address = f as u128;
        let target = address & 0xffff | ((address & 0xffff_0000) << 32) | ((address & 0xffff_ffff_0000_0000) << 32);
        let segment = 1 << 19;

        // Target offset + kernel code segment + 64-bit interrupt gate + present.
        Self(target | segment | 0b1110 << 40 | 1 << 47, PhantomData)
    }
}
