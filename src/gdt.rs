use core::{arch::asm, mem::size_of};

#[derive(Debug)]
#[repr(transparent)]
struct GlobalDescriptorTable([Descriptor; 5]);

#[derive(Debug)]
#[repr(transparent)]
struct Descriptor(u64);

#[derive(Debug)]
#[repr(C, packed)]
struct GdtPointer {
    size: u16,
    ptr: *const Descriptor
}

// This is marked as mutable because the CPU is able to write to the GDT.
static mut GDT: GlobalDescriptorTable = GlobalDescriptorTable::new();

/// Loads the GDTR with the kernel's global descriptor table.
pub fn load() {
    unsafe {
        GDT.load();
    }
}

impl Descriptor {
    /// Creates a null memory segment descriptor.
    const fn new_null() -> Self {
        Self(0)
    }

    /// Creates a kernel-mode code memory segment descriptor.
    const fn new_kernel_code() -> Self {
        // Readable + code descriptor + DPL 0 + present + long mode.
        Self(1 << 41 | 0b11 << 43 | 1 << 47 | 1 << 53)
    }

    /// Creates a user-mode code memory segment descriptor.
    const fn new_user_code() -> Self {
        // Readable + code descriptor + DPL 3 + present + long mode.
        Self(1 << 41 | 0b11 << 43 | 0b11 << 45 | 1 << 47 | 1 << 53)
    }

    /// Creates a kernel data memory segment descriptor.
    const fn new_kernel_data() -> Self {
        // Readable + data descriptor + DPL 0 + present + long mode.
        Self(1 << 41 | 0b10 << 43 | 1 << 47)
    }

    /// Creates a user-mode data memory segment descriptor.
    const fn new_user_data() -> Self {
        // Readable + data descriptor + DPL 3 + present + long mode.
        Self(1 << 41 | 0b10 << 43 | 0b11 << 45 | 1 << 47)
    }
}

impl GlobalDescriptorTable {
    /// Creates a pre-populated global descriptor table suitable for use in DPL 0.
    const fn new() -> Self {
        Self([
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
    /// Callers must ensure that:
    /// - A valid null descriptor exists at offset 0 (in bytes).
    /// - A valid 64-bit DPL 0 code descriptor exists at offset 8 (in bytes).
    /// - A valid 64-bit DPL 0 data descriptor exists at offset 16 (in bytes).
    /// - A valid 64-bit DPL 3 code descriptor exists at offset 24 (in bytes).
    /// - A valid 64-bit DPL 3 code descriptor exists at offset 32 (in bytes).
    unsafe fn load(&'static self) {
        let ptr = GdtPointer {
            size: (size_of::<GlobalDescriptorTable>() - 1) as u16,
            ptr: self.0.as_ptr()
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
