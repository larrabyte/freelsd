use core::arch::asm;

/// Indicates a type that can be read from or written to an I/O port.
pub trait Blob {
    /// Read a value in from an I/O port.
    /// 
    /// # Safety
    /// Reading values from an I/O port can have arbitrary side effects.
    unsafe fn read(port: u16) -> Self;

    /// Write a value to an I/O port.
    /// 
    /// # Safety
    /// Writing values to an I/O port can have arbitrary side effects.
    unsafe fn write(port: u16, value: Self);
}

impl Blob for u8 {
    unsafe fn read(port: u16) -> Self {
        let value: Self;

        asm!(
            "in al, dx",
            out("al") value, 
            in("dx") port,
            options(nomem, nostack, preserves_flags)
        );

        value
    }

    unsafe fn write(port: u16, value: Self) {
        asm!(
            "out dx, al",
            in("dx") port,
            in("al") value,
            options(nomem, nostack, preserves_flags)
        );
    }
}

impl Blob for u16 {
    unsafe fn read(port: u16) -> Self {
        let value: Self;

        asm!(
            "in ax, dx",
            out("ax") value, 
            in("dx") port,
            options(nomem, nostack, preserves_flags)
        );

        value
    }

    unsafe fn write(port: u16, value: Self) {
        asm!(
            "out dx, ax",
            in("dx") port,
            in("ax") value,
            options(nomem, nostack, preserves_flags)
        );
    }
}

impl Blob for u32 {
    unsafe fn read(port: u16) -> Self {
        let value: Self;

        asm!(
            "in eax, dx",
            out("eax") value, 
            in("dx") port,
            options(nomem, nostack, preserves_flags)
        );

        value
    }

    unsafe fn write(port: u16, value: Self) {
        asm!(
            "out dx, eax",
            in("dx") port,
            in("eax") value,
            options(nomem, nostack, preserves_flags)
        );
    }
}
