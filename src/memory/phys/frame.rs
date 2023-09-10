use crate::boot::HHDM;
use core::ptr::NonNull;

/// A physical memory frame, representing a region of physical memory 4096 bytes in size.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
#[repr(transparent)]
pub struct Frame(NonNull<u8>);

impl Frame {
    /// The number of bytes that a frame spans.
    pub const SIZE: usize = 0x1000;

    /// Creates a physical frame containing `addr`.
    ///
    /// # Panics
    /// - If `addr` is zero.
    pub fn containing_addr(addr: usize) -> Self {
        Self(NonNull::new((addr & !(Frame::SIZE - 1)) as *mut u8).expect("zero is not a valid frame"))
    }

    /// Creates a physical frame containing the address pointed to by `ptr`.
    ///
    /// # Panics
    /// - If `ptr` is null.
    pub fn containing_ptr(ptr: *mut u8) -> Self {
        let hhdm = HHDM.get_response().get().unwrap();
        Self::containing_addr(ptr as usize - hhdm.offset as usize)
    }

    /// Returns the physical address of this frame.
    pub fn addr(&self) -> usize {
        self.0.as_ptr() as usize
    }

    /// Creates a valid, dereferenceable virtual pointer to this frame.
    pub fn as_non_null(&self) -> NonNull<u8> {
        let hhdm = HHDM.get_response().get().unwrap();
        let ptr = (self.addr() + hhdm.offset as usize) as *mut u8;
        NonNull::new(ptr).unwrap()
    }
}
