mod traits;

use core::marker::PhantomData;
use self::traits::Blob;

/// An x86_64 I/O port, which can read and/or write values of `T`.
#[derive(Debug)]
pub struct UnsafePort<T: Blob> {
    port: u16,
    phantom: PhantomData<T>
}

impl<T: Blob> UnsafePort<T> {
    /// Creates an instance of `UnsafePort` for reading/writing values of `T`.
    pub const unsafe fn new(port: u16) -> Self {
        Self {port, phantom: PhantomData}
    }

    /// Reads a value of `T` from an I/O port.
    /// 
    /// # Safety
    /// See [`traits::Blob`].
    pub unsafe fn read(&mut self) -> T {
        T::read(self.port)
    }

    /// Writes a value of `T` to an I/O port.
    /// 
    /// # Safety
    /// See [`traits::Blob`].
    pub unsafe fn write(&mut self, value: T) {
        T::write(self.port, value);
    }
}
