use core::{arch::asm, mem::size_of};

#[derive(Debug)]
#[repr(transparent)]
struct Descriptor(u64);

#[derive(Debug)]
#[repr(transparent)]
pub struct GlobalDescriptorTable([Descriptor; 5]);

#[derive(Debug)]
#[repr(C, packed)]
struct GdtPointer {
    size: u16,
    pointer: *const Descriptor
}

impl Descriptor {
    /// Creates a null memory segment descriptor.
    const fn new_null() -> Self {
        Descriptor(0)
    }

    /// Creates a kernel-mode code memory segment descriptor.
    const fn new_kernel_code() -> Self {
        // Readable + code descriptor + DPL 0 + present + long mode.
        Descriptor(1 << 41 | 0b11 << 43 | 1 << 47 | 1 << 53)
    }

    /// Creates a user-mode code memory segment descriptor.
    const fn new_user_code() -> Self {
        // Readable + code descriptor + DPL 3 + present + long mode.
        Descriptor(1 << 41 | 0b11 << 43 | 0b11 << 45 | 1 << 47 | 1 << 53)
    }

    /// Creates a kernel data memory segment descriptor.
    const fn new_kernel_data() -> Self {
        // Readable + data descriptor + DPL 0 + present + long mode.
        Descriptor(1 << 41 | 0b10 << 43 | 1 << 47)
    }

    /// Creates a user-mode data memory segment descriptor.
    const fn new_user_data() -> Self {
        // Readable + data descriptor + DPL 3 + present + long mode.
        Descriptor(1 << 41 | 0b10 << 43 | 0b11 << 45 | 1 << 47)
    }
}

impl GlobalDescriptorTable {
    /// Creates a new global descriptor table.
    pub const fn new() -> Self {
        GlobalDescriptorTable([
            Descriptor::new_null(),
            Descriptor::new_kernel_code(),
            Descriptor::new_kernel_data(),
            Descriptor::new_user_code(),
            Descriptor::new_user_data()
        ])
    }

    /// Loads the GDTR with this global descriptor table.
    ///
    /// # Safety
    /// The GDTR should only be loaded during kernel initialisation.
    pub unsafe fn load(&mut self) {
        let ptr = GdtPointer {
            size: (size_of::<GlobalDescriptorTable>() - 1) as u16,
            pointer: self.0.as_ptr()
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
            inout("rax") &ptr => _,
            options(preserves_flags)
        );
    }
}
