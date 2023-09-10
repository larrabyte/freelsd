use core::ptr::NonNull;

/// A virtual memory page, representing a region of virtual memory 4096 bytes in size.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
#[repr(transparent)]
pub struct Page(NonNull<u8>);

impl Page {
    /// The number of bytes that a page spans.
    pub const SIZE: usize = 0x1000;

    /// Creates a virtual page containing `addr`, unless it resides in the first page.
    pub fn containing_addr(addr: usize) -> Self {
        Self(NonNull::new((addr & !(Page::SIZE - 1)) as *mut u8).expect("zero is not a valid page")) 
    }

    /// Returns the virtual address of this page.
    pub fn addr(&self) -> usize {
        self.0.as_ptr() as usize
    }

    /// Returns the 9-bit page-map level 4 index for this page.
    pub fn pml4_index(&self) -> usize {
        (self.addr() >> 39) & 0o777
    }

    /// Returns the 9-bit page directory pointer table index for this page.
    pub fn pdpt_index(&self) -> usize {
        (self.addr() >> 30) & 0o777
    }

    /// Returns the 9-bit page directory table index for this page.
    pub fn pdt_index(&self) -> usize {
        (self.addr() >> 21) & 0o777
    }

    /// Returns the 9-bit page table index for this page.
    pub fn pt_index(&self) -> usize {
        (self.addr() >> 12) & 0o777
    }
}
