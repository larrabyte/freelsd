use crate::memory::phys::{frame::Frame, allocator::PHYSICAL_BITMAP_ALLOCATOR as PHYS};
use core::{ops::{Index, IndexMut, BitOr, BitAnd, BitXor}, marker::PhantomData};

/// A representation of a page table.
#[repr(align(4096))]
pub struct PageTable<L: PageLevel>([PageEntry<L>; 512]);

/// A 64-bit page table entry.
#[repr(transparent)]
pub struct PageEntry<L: PageLevel>(u64, PhantomData<L>);

/// Equivalent to a Page-Map Level-4 on `x86_64`.
pub struct Level4;

/// Equivalent to a Page Directory Pointer on `x86_64`.
pub struct Level3;

/// Equivalent to a Page Directory on `x86_64`.
pub struct Level2;

/// Equivalent to a Page Table on `x86_64`.
pub struct Level1;

/// A representation of the flags available in a page table entry.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
#[repr(transparent)]
pub struct PageEntryFlags(u64);

/// A marker trait for page table entry levels.
pub trait PageLevel {
    type Underlying;
}

impl<L: PageLevel> PageTable<L> {
    /// Creates a new, zeroed page table.
    pub fn new() -> Self {
        Self(core::array::from_fn(|_| PageEntry(0, PhantomData)))
    }
}

impl<L: PageLevel> PageEntry<L> {
    /// The bitmask for the address stored in this entry.
    const ADDRESS_MASK: u64 = 0xf_ffff_ffff_f000;

    /// Returns the set of flags currently set for this entry.
    pub fn flags(&self) -> PageEntryFlags {
        PageEntryFlags::from_bits(self.0)
    }

    /// Returns the numerical address of the frame that this entry controls.
    pub fn addr(&self) -> usize {
        (self.0 & Self::ADDRESS_MASK) as usize
    }

    /// Returns the frame that this entry controls, if any.
    pub fn frame(&self) -> Option<Frame> {
        match self.flags().contains(PageEntryFlags::PRESENT) {
            true => Some(Frame::containing_addr(self.addr())),
            false => None
        }
    }

    /// Swaps the frame and flags (if any exist) currently mapped by this entry, returning the
    /// frame that was previously mapped.
    ///
    /// # Safety
    /// Callers must ensure that a frame swap does not violate assumptions made by any variables
    /// with references to memory controlled by this entry.
    pub unsafe fn swap(&mut self, frame: Option<Frame>, flags: PageEntryFlags) -> Option<Frame> {
        let addr = frame.map(|f| f.addr()).unwrap_or(0);

        if addr == 0 && flags.contains(PageEntryFlags::PRESENT) {
            panic!("attempted to mark page table entry as present with null frame");
        }

        let new = addr as u64 | flags.0;
        let old = core::mem::replace(&mut self.0, new);

        match old & Self::ADDRESS_MASK {
            0 => None,
            addr => Some(Frame::containing_addr(addr as usize))
        }
    }

    /// Generates an optional reference to the underlying type referenced by this entry.
    pub fn underlying(&self) -> Option<&L::Underlying> {
        match self.flags().contains(PageEntryFlags::PRESENT) {
            // If the entry is marked as present, then we assume initialisation and alignment.
            // The exclusive borrow generated has the same lifetime as the entry.
            true => unsafe {
                let frame = Frame::containing_addr(self.addr());
                Some(frame.as_non_null().cast::<L::Underlying>().as_ref())
            },

            false => None
        }
    }

    /// Generates an optional mutable reference to the underlying type referenced by this entry.
    pub fn underlying_mut(&mut self) -> Option<&mut L::Underlying> {
        match self.flags().contains(PageEntryFlags::PRESENT) {
            // If the entry is marked as present, then we assume initialisation and alignment.
            // The exclusive borrow generated has the same lifetime as the entry.
            true => unsafe {
                let frame = Frame::containing_addr(self.addr());
                Some(frame.as_non_null().cast::<L::Underlying>().as_mut())
            },

            false => None
        }
    }

    /// Generate a mutable reference to the underlying type, either using the reference in the
    /// entry if marked as present, or by allocating a new instance and marking the entry as
    /// present.
    ///
    /// # Panics
    /// - If a page table allocation is required and physical memory has been exhausted.
    pub fn underlying_mut_or_allocate(&mut self) -> &mut L::Underlying {
        match self.flags().contains(PageEntryFlags::PRESENT) {
            // If the entry is marked as present, then we assume initialisation and alignment.
            // The exclusive borrow generated has the same lifetime as the entry.
            true => unsafe {
                let frame = Frame::containing_addr(self.addr());
                frame.as_non_null().cast::<L::Underlying>().as_mut()
            },

            // The frame is aligned and initialised by the frame allocator.
            // The mutable reference can only be generated if the caller already has a mutable
            // reference to this entry.
            false => unsafe {
                let frame = PHYS.lock().alloc_zeroed().expect("page tables require physical memory");
                self.swap(Some(frame), PageEntryFlags::PRESENT | PageEntryFlags::WRITABLE);
                frame.as_non_null().cast::<L::Underlying>().as_mut()
            }
        }
    }
}

impl PageEntryFlags {
    /// Indicates whether this entry is present.
    pub const PRESENT: Self = Self(1 << 0);

    /// Indicateds whether this entry is writable.
    pub const WRITABLE: Self = Self(1 << 1);

    /// Indicates whether this entry is accessible from userspace.
    pub const USER_ACCESSIBLE: Self = Self(1 << 2);

    /// Indicates whether this entry has a writethrough caching policy.
    pub const WRITETHROUGH: Self = Self(1 << 3);

    /// Indicates whether this entry is cacheable.
    pub const CACHEABLE: Self = Self(1 << 4);

    /// FreeLSD-specific bit to indicate a shared frame. These are not released to the physical
    /// memory allocator when an address space is dropped.
    pub const SHARED: Self = Self(1 << 9);

    /// Creates an empty set of flags.
    pub fn none() -> Self {
        Self(0)
    }

    /// Creates a set of flags from the `x86_64` bitwise page table entry representation.
    /// Set bits that do not have any associated meaning are ignored.
    pub fn from_bits(bits: u64) -> Self {
        Self(bits & 0b1000011111)
    }

    /// Checks whether `flags` is contained in this set of flags.
    pub fn contains(&self, flags: Self) -> bool {
        (self.0 & flags.0) == flags.0
    }
}

impl PageLevel for Level4 {
    type Underlying = PageTable<Level3>;
}

impl PageLevel for Level3 {
    type Underlying = PageTable<Level2>;
}

impl PageLevel for Level2 {
    type Underlying = PageTable<Level1>;
}

impl PageLevel for Level1 {
    type Underlying = ();
}

impl<L: PageLevel> Index<usize> for PageTable<L> {
    type Output = PageEntry<L>;

    fn index(&self, index: usize) -> &Self::Output {
        &self.0[index]
    }
}

impl<L: PageLevel> IndexMut<usize> for PageTable<L> {
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        &mut self.0[index]
    }
}

impl<L: PageLevel> core::fmt::Debug for PageEntry<L> {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        // Print page table entries in hexadecimal by default.
        f.debug_tuple("PageEntry").field(&format_args!("{:#x}", self.0)).finish()
    }
}

impl<L: PageLevel> Drop for PageEntry<L> {
    fn drop(&mut self) {
        if self.flags().contains(PageEntryFlags::PRESENT) && !self.flags().contains(PageEntryFlags::SHARED) {
            // This entry is marked as unshared, therefore no other address space should have a mapping for this frame.
            // An exclusive borrow of the page entry also means no other code has a reference, so
            // dropping in place is OK.
            unsafe {
                core::ptr::drop_in_place(self.underlying_mut().unwrap());
                PHYS.lock().dealloc(self.frame().unwrap())
            }
        }
    }
}

impl BitOr for PageEntryFlags {
    type Output = Self;

    fn bitor(self, rhs: Self) -> Self::Output {
        Self(self.0 | rhs.0)
    }
}

impl BitAnd for PageEntryFlags {
    type Output = Self;

    fn bitand(self, rhs: Self) -> Self::Output {
        Self(self.0 & rhs.0)
    }
}

impl BitXor for PageEntryFlags {
    type Output = Self;

    fn bitxor(self, rhs: Self) -> Self::Output {
        Self(self.0 ^ rhs.0)
    }
}
