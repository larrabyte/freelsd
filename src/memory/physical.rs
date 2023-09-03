#![allow(dead_code)]

use crate::boot::MEMORY_MAP;
use core::ptr::NonNull;
use limine::MemoryMapEntryType;
use spin::{Lazy, Mutex};

#[derive(Debug)]
pub struct PhysicalMemoryAllocator {
    bitmap: &'static mut [u8]
}

const FRAME_SIZE: usize = 0x1000;

pub static PHYSICAL_BITMAP_ALLOCATOR: Lazy<Mutex<PhysicalMemoryAllocator>> = Lazy::new(|| {
    // TODO: Incorporate bootloader reclaimable memory.
    let memory = MEMORY_MAP.get_response().get().unwrap();

    // Since the Limine boot protocol only guarantees that usable and bootloader reclaimable
    // entries are non-overlapping and page-aligned, we must find the last available address
    // and use it to calculate the resulting size of the bitmap that covers 0..=end.
    let end = memory.memmap().iter().rev()
        .find(|e| e.typ == MemoryMapEntryType::Usable /* || e.typ == MemoryMapEntryType::BootloaderReclaimable */)
        .map(|e| (e.base + e.len) as usize)
        .expect("physical memory allocator requires memory to allocate");

    // Perform ceiling division to account for imprecise division.
    let frames = (end + FRAME_SIZE - 1) / FRAME_SIZE;
    let bytes = (frames + 7) / 8;

    let base = memory.memmap().iter()
        .find(|e| e.typ == MemoryMapEntryType::Usable /* || e.typ == MemoryMapEntryType::BootloaderReclaimable */ && e.len as usize > bytes)
        .map(|e| e.base as *mut u8)
        .expect("physical memory allocator requires sufficient storage");

    if bytes > isize::MAX as usize {
        panic!("physical memory allocator requires contiguous storage");
    }

    // The pointer we use to initialise the slice points to a contiguous,
    // sufficiently large section of memory, is aligned and the memory it
    // points to is only ever used via the slice. The memory is also initialised
    // immediately before it is used to create the slice.
    let bitmap = unsafe {
        for byte in 0..bytes {
            let result = (0..8)
                .map(|i| {
                    memory
                        .memmap()
                        .iter()
                        .filter(|e| e.typ == MemoryMapEntryType::Usable /* || e.typ == MemoryMapEntryType::BootloaderReclaimable */)
                        .find(|e| ((e.base as usize)..(e.base as usize + e.len as usize)).contains(&((byte * 8 + i) * FRAME_SIZE)))
                        .is_some() as u8
                })

                .enumerate()
                .fold(0xFF, |acc, (index, bit)| acc ^ (bit << (7 - index)));

            base.add(byte).write(result);
        }

        core::slice::from_raw_parts_mut(base, bytes)
    };

    Mutex::new(PhysicalMemoryAllocator { bitmap })
});

impl PhysicalMemoryAllocator {
    /// Attempts to allocate a single frame of physical memory.
    pub fn allocate(&mut self) -> Option<NonNull<u8>> {
        let (index, byte) = self.bitmap.iter_mut().enumerate().find(|(_, b)| **b != 0xFF)?;
        let offset = (0..8).rev().map(|v| *byte & (1 << v)).position(|b| b == 0).unwrap();
        *byte |= 1 << (7 - offset);

        NonNull::new(((index * 8 + offset as usize) * FRAME_SIZE) as *mut u8)
    }

    /// Deallocates a frame of physical memory.
    ///
    /// # Safety
    /// Callers must ensure that the given frame is currently allocated via this allocator and that
    /// deallocated memory is not read from or written to afterwards.
    pub unsafe fn deallocate(&mut self, memory: NonNull<u8>) {
        let frame = memory.as_ptr() as usize / FRAME_SIZE;
        self.bitmap[frame / 8] &= !(1 << (7 - frame % 8));
    }
}

#[cfg(test)]
mod test {
    use super::PHYSICAL_BITMAP_ALLOCATOR;

    #[test_case]
    fn allocator_returns_different_frames_if_memory_is_available() {
        let a = PHYSICAL_BITMAP_ALLOCATOR.lock().allocate();
        let b = PHYSICAL_BITMAP_ALLOCATOR.lock().allocate();

        match (a, b) {
            // If no memory is available, then two Nones is okay.
            (None, None) => {},

            // Otherwise, check whether the allocations are disjoint.
            _ => assert_ne!(a, b)
        };
    }
}
