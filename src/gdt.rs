use core::{arch::asm, mem::size_of};

#[derive(Debug)]
#[repr(transparent)]
struct Descriptor(u64);

#[derive(Debug)]
#[repr(C, packed)]
struct GdtPointer {
    size: u16,
    pointer: *const Descriptor
}

#[derive(Debug)]
enum DescriptorPrivilegeLevel {
    Kernel = 0b00,
    User = 0b11
}

// The CPU writes to the GDT, even though we don't.
static mut GDT: [Descriptor; 5] = [
    Descriptor::new_null(),
    Descriptor::new_code_with_dpl(DescriptorPrivilegeLevel::Kernel),
    Descriptor::new_data_with_dpl(DescriptorPrivilegeLevel::Kernel),
    Descriptor::new_code_with_dpl(DescriptorPrivilegeLevel::User),
    Descriptor::new_data_with_dpl(DescriptorPrivilegeLevel::User)
];

impl Descriptor {
    const fn new_null() -> Self {
        Self(0)
    }

    /// Creates a 64-bit code descriptor with a given privilege level.
    const fn new_code_with_dpl(level: DescriptorPrivilegeLevel) -> Self {
        // DPL + readable + code descriptor + present + long.
        Self((level as u64) << 45 | 1 << 41 | 0b11 << 43 | 1 << 47 | 1 << 53)
    }

    /// Creates a 64-bit data descriptor with a given privilege level.
    const fn new_data_with_dpl(level: DescriptorPrivilegeLevel) -> Self {
        // DPL + data descriptor + present.
        Self((level as u64) << 45 | 1 << 41 | 0b10 << 43 | 1 << 47)
    }
}

/// Loads the GDTR with the kernel's GDT.
/// 
/// # Safety
/// The GDTR should only be loaded during kernel initialisation.
pub unsafe fn load() {
    let pointer = GdtPointer {
        size: (GDT.len() * size_of::<Descriptor>() - 1) as u16,
        pointer: GDT.as_ptr()
    };

    asm!(
        "lgdt [rax]",
        "push 0x08",
        "lea rax, [2f]",
        "push rax",
        "retfq",
        "2:",
        "mov ax, 0x10",
        "mov ds, ax",
        "mov es, ax",
        "mov fs, ax",
        "mov gs, ax",
        "mov ss, ax",
        inout("rax") &pointer => _,
        options(preserves_flags)
    );
}
