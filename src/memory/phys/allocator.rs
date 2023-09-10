use super::frame::Frame;
use crate::boot::MEMORY_MAP;
use limine::MemoryMapEntryType;
use spin::{Lazy, Mutex};

/// A bitmap-based physical frame allocator.
pub struct PhysicalFrameAllocator(&'static mut [u8]);

pub static PHYSICAL_BITMAP_ALLOCATOR: Lazy<Mutex<PhysicalFrameAllocator>> = Lazy::new(|| {
    // TODO: Incorporate bootloader reclaimable memory.
    let memory = MEMORY_MAP.get_response().get().unwrap();

    // Since the Limine boot protocol only guarantees that usable and bootloader reclaimable
    // entries are non-overlapping and aligned to a 4KiB boundary, we must find the last available address
    // and use it to calculate the resulting size of the bitmap that covers 0..=end.
    let end = memory.memmap().iter().rev()
        .find(|e| e.typ == MemoryMapEntryType::Usable /* || e.typ == MemoryMapEntryType::BootloaderReclaimable */)
        .map(|e| (e.base + e.len) as usize)
        .expect("physical memory allocator requires memory to allocate");

    // Perform ceiling division to account for imprecise division.
    let frames = (end + Frame::SIZE - 1) / Frame::SIZE;
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
            base.add(byte).write(0xFF);
        }

        for entry in memory.memmap().iter().filter(|e| e.typ == MemoryMapEntryType::Usable /* || e.typ == MemoryMapEntryType::BootloaderReclaimable */) {
            let start = (entry.base as usize + Frame::SIZE - 1) / Frame::SIZE;
            let end = (entry.base as usize + entry.len as usize) / Frame::SIZE;

            for frame in start..end {
                let index = frame / 8;
                let offset = frame % 8;
                let byte = base.add(index).read() & !(1 << (7 - offset));
                base.add(index).write(byte);
            }
        }

        core::slice::from_raw_parts_mut(base, bytes)
    };

    Mutex::new(PhysicalFrameAllocator(bitmap))
});

impl PhysicalFrameAllocator {
    /// Checks whether `frame` is currently marked as available for allocation.
    pub fn available(&self, frame: Frame) -> bool {
        let index = frame.addr() / Frame::SIZE;
        self.0[index / 8] & (1 << (7 - index % 8)) == 0
    }

    /// Allocates a frame of physical memory, unless memory has been exhausted.
    pub fn alloc(&mut self) -> Option<Frame> {
        let (index, byte) = self.0.iter_mut().enumerate().find(|(_, b)| **b != 0xFF)?;
        let offset = (0..8).rev().map(|v| *byte & (1 << v)).position(|b| b == 0).unwrap();
        *byte |= 1 << (7 - offset);

        Some(Frame::containing_addr(((index * 8) + offset) * Frame::SIZE))
    }

    /// Allocates a zeroed frame of physical memory, unless memory has been exhausted.
    pub fn alloc_zeroed(&mut self) -> Option<Frame> {
        let frame = self.alloc()?;
        let ptr = frame.as_non_null().as_ptr();

        // The pointer should be dereferenceable as it came from a physical frame allocation,
        // and it does not leave the bounds of the frame.
        unsafe {
            for i in 0..Frame::SIZE {
                ptr.add(i).write(0x00);
            }
        }

        Some(frame)
    }

    /// Deallocates a frame of physical memory.
    ///
    /// # Safety
    /// Callers must ensure that `frame` was allocated by the same allocator instance and that no
    /// further reads or writes occur after deallocation.
    pub unsafe fn dealloc(&mut self, frame: Frame) {
        let index = frame.addr() / Frame::SIZE;
        self.0[index / 8] &= !(1 << (7 - index % 8));
    }
}

#[cfg(test)]
mod test {
    use super::PHYSICAL_BITMAP_ALLOCATOR as ALLOCATOR;

    #[test_case]
    fn allocator_marks_allocated_frames_as_unavailable() {
        let frame = ALLOCATOR.lock().alloc().unwrap();
        assert_eq!(ALLOCATOR.lock().available(frame), false);
    }

    #[test_case]
    fn allocator_allocates_disjoint_frames() {
        let a = ALLOCATOR.lock().alloc().unwrap();
        let b = ALLOCATOR.lock().alloc().unwrap();
        assert_ne!(a, b);
    }

    #[test_case]
    fn allocator_marks_deallocated_frames_as_available() {
        let frame = ALLOCATOR.lock().alloc().unwrap();

        unsafe {
            ALLOCATOR.lock().dealloc(frame);
        }

        assert_eq!(ALLOCATOR.lock().available(frame), true);
    }
}
