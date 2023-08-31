use self::handlers::{panic_without_error_code, panic_with_error_code};
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
    reserved_31: InterruptGate<WithoutErrorCode>,
    reserved_32_to_255: [InterruptGate<WithoutErrorCode>; 224]
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
    pointer: *const InterruptDescriptorTable
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
            division_by_zero: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            debug: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            non_maskable_interrupt: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            breakpoint: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            overflow: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            bound_range_exceeded: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            invalid_opcode: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            device_not_available: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            double_fault: InterruptGate::<WithErrorCode>::new(panic_with_error_code),
            coprocessor_segment_overrun: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            invalid_task_state_segment: InterruptGate::<WithErrorCode>::new(panic_with_error_code),
            segment_not_present: InterruptGate::<WithErrorCode>::new(panic_with_error_code),
            stack_segment_fault: InterruptGate::<WithErrorCode>::new(panic_with_error_code),
            general_protection_fault: InterruptGate::<WithErrorCode>::new(panic_with_error_code),
            page_fault: InterruptGate::<WithErrorCode>::new(panic_with_error_code),
            reserved_15: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            x87_floating_point_exception: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            alignment_check: InterruptGate::<WithErrorCode>::new(panic_with_error_code),
            machine_check: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            simd_floating_point_exception: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            virtualisation_exception: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            control_protection_exception: InterruptGate::<WithErrorCode>::new(panic_with_error_code),
            reserved_22_to_27: [InterruptGate::<WithoutErrorCode>::new(panic_without_error_code); 6],
            hypervisor_injection_exception: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            vmm_communication_exception: InterruptGate::<WithErrorCode>::new(panic_with_error_code),
            security_exception: InterruptGate::<WithErrorCode>::new(panic_with_error_code),
            reserved_31: InterruptGate::<WithoutErrorCode>::new(panic_without_error_code),
            reserved_32_to_255: [InterruptGate::<WithoutErrorCode>::new(panic_without_error_code); 224]
        }
    }

    /// Loads the IDTR with this interrupt descriptor table.
    ///
    /// # Safety
    /// Callers must ensure that all interrupt gates are present, marked as 64-bit and set to DPL 0.
    unsafe fn load(&'static self) {
        let ptr = IdtPointer {
            size: (size_of::<InterruptDescriptorTable>() - 1) as u16,
            pointer: self as *const InterruptDescriptorTable
        };

        asm!("lidt [{}]", in(reg) &ptr, options(preserves_flags));
    }
}

impl InterruptGate<WithoutErrorCode> {
    fn new(f: unsafe extern "x86-interrupt" fn(InterruptStackFrame)) -> Self {
        let address = f as u128;
        let target = address & 0xFFFF | ((address & 0xFFFF0000) << 32) | ((address & 0xFFFFFFFF00000000) << 32);
        let segment = 1 << 19;

        // Target offset + kernel code segment + 64-bit interrupt gate + present.
        Self(target | segment | 0b1110 << 40 | 1 << 47, PhantomData)
    }
}

impl InterruptGate<WithErrorCode> {
    fn new(f: unsafe extern "x86-interrupt" fn(InterruptStackFrame, u64)) -> Self {
        let address = f as u128;
        let target = address & 0xFFFF | ((address & 0xFFFF0000) << 32) | ((address & 0xFFFFFFFF00000000) << 32);
        let segment = 1 << 19;

        // Target offset + kernel code segment + 64-bit interrupt gate + present.
        Self(target | segment | 0b1110 << 40 | 1 << 47, PhantomData)
    }
}
