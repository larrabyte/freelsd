use limine::{BootInfoRequest, MemmapRequest};

pub static BOOTLOADER_INFORMATION: BootInfoRequest = BootInfoRequest::new(0);
pub static MEMORY_MAP: MemmapRequest = MemmapRequest::new(0);
