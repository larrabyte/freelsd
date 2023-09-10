use super::{page::Page, tables::{PageTable, Level4, PageEntryFlags}};
use crate::memory::phys::{frame::Frame, allocator::PHYSICAL_BITMAP_ALLOCATOR as PHYS};
use core::{ops::{IndexMut, Index}, ptr::NonNull};

#[derive(Debug)]
pub struct AddressSpace {
    // This table is owned by the address space,
    // but it shouldn't be stored on the stack.
    tables: Option<NonNull<PageTable<Level4>>>
}

impl AddressSpace {
    /// Creates an empty address space.
    pub fn new() -> Self {
        Self { tables: None }
    }

    /// Translates a virtual page into a physical frame (which might not exist).
    pub fn translate(&self, addr: Page) -> Option<Frame> {
        // Assume a non-null pointer has suitable alignment and is initialised.
        let tables = unsafe {
            self.tables?.as_ref()
        };

        tables
            .index(addr.pml4_index())
            .underlying()?
            .index(addr.pdpt_index())
            .underlying()?
            .index(addr.pdt_index())
            .underlying()?
            .index(addr.pt_index())
            .frame()
    }

    /// Attempts to map `addr` to `frame`, returning the previously mapped frame if it exists.
    ///
    /// # Safety
    /// See the `swap` method on [`super::tables::PageEntry`].
    pub unsafe fn map(&mut self, virt: Page, phys: Option<Frame>, flags: PageEntryFlags) -> Option<Frame> {
        let tables = match self.tables {
            // We have an exclusive borrow of the address space.
            Some(mut table) => {
                table.as_mut()
            },

            // No one can reference a newly-allocated frame.
            None => {
                let frame = PHYS.lock().alloc_zeroed().expect("page tables require physical memory");
                self.tables = Some(frame.as_non_null().cast::<PageTable<Level4>>());
                frame.as_non_null().cast::<PageTable<Level4>>().as_mut()
            }
        };

        let pte = tables
            .index_mut(virt.pml4_index())
            .underlying_mut_or_allocate()
            .index_mut(virt.pdpt_index())
            .underlying_mut_or_allocate()
            .index_mut(virt.pdt_index())
            .underlying_mut_or_allocate()
            .index_mut(virt.pt_index());

        // Safety is upheld by the caller.
        pte.swap(phys, flags)
    }
}

impl Drop for AddressSpace {
    fn drop(&mut self) {
        if let Some(table) = self.tables {
            // We have an exclusive borrow of the address space, and the page table itself does not call
            // any other drop implementation since it is behind a pointer.
            unsafe {
                let ptr = table.as_ptr();
                core::ptr::drop_in_place(ptr);
                PHYS.lock().dealloc(Frame::containing_ptr(ptr as *mut u8));
            }
        }
    }
}

#[cfg(test)]
mod test {
    use super::AddressSpace;
    use crate::memory::virt::{page::Page, tables::PageEntryFlags};
    use crate::memory::phys::allocator::PHYSICAL_BITMAP_ALLOCATOR as PHYS;

    #[test_case]
    fn address_space_maps_pages_to_frames() {
        let mut space = AddressSpace::new();
        let addr = Page::containing_addr(0x1000);
        let frame = PHYS.lock().alloc().unwrap();

        // 0x1000 has no mapping.
        unsafe {
            space.map(addr, Some(frame), PageEntryFlags::PRESENT | PageEntryFlags::WRITABLE);
        }

        assert_ne!(space.translate(addr), None);
    }

    #[test_case]
    fn allocator_backs_pages_with_disjoint_frames() {
        let mut space = AddressSpace::new();

        let alpha = (
            Page::containing_addr(0x1000),
            PHYS.lock().alloc().unwrap()
        );

        let beta = (
            Page::containing_addr(0x2000),
            PHYS.lock().alloc().unwrap()
        );

        // 0x1000 and 0x2000 have no mapped frames.
        unsafe {
            space.map(alpha.0, Some(alpha.1), PageEntryFlags::PRESENT | PageEntryFlags::WRITABLE);
            space.map(beta.0, Some(beta.1), PageEntryFlags::PRESENT | PageEntryFlags::WRITABLE);
        }

        assert_ne!(space.translate(alpha.0).unwrap(), space.translate(beta.0).unwrap());
    }

    #[test_case]
    fn address_space_unmaps_allocated_pages() {
        let mut space = AddressSpace::new();
        let addr = Page::containing_addr(0x1000);
        let frame = PHYS.lock().alloc().unwrap();

        // 0x1000 has no mapping.
        unsafe {
            space.map(addr, Some(frame), PageEntryFlags::PRESENT | PageEntryFlags::WRITABLE);
        }

        // The address space is completely unused.
        let free = unsafe {
            space.map(addr, None, PageEntryFlags::none())
        };

        assert_eq!(Some(frame), free);
        assert_eq!(space.translate(addr), None);
    }

    #[test_case]
    fn address_space_releases_unshared_frames() {
        let mut space = AddressSpace::new();
        let addr = Page::containing_addr(0x1000);
        let frame = PHYS.lock().alloc().unwrap();

        // 0x1000 has no mapping.
        unsafe {
            space.map(addr, Some(frame), PageEntryFlags::PRESENT | PageEntryFlags::WRITABLE);
        }

        core::mem::drop(space);
        assert_eq!(PHYS.lock().available(frame), true);
    }
}
