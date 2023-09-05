use core::ptr::NonNull;

/// A physical memory frame, representing a region of physical memory 4096 bytes in size.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
#[repr(transparent)]
pub struct Frame(NonNull<()>);

impl Frame {
    /// Attempts to create a new physical frame.
    ///
    /// # Errors
    /// - Returns `None` if `addr` is zero.
    pub fn new(addr: usize) -> Option<Self> {
        Some(Self(NonNull::new(addr as *mut ())?))
    }

    /// Returns the numerical address of this physical frame.
    pub fn addr(&self) -> usize {
        self.0.as_ptr() as usize
    }
}
