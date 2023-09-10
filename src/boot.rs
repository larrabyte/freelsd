use limine::{BootInfoRequest, MemmapRequest, HhdmRequest};

pub static BOOTLOADER_INFORMATION: BootInfoRequest = BootInfoRequest::new(0);
pub static MEMORY_MAP: MemmapRequest = MemmapRequest::new(0);
pub static HHDM: HhdmRequest = HhdmRequest::new(0);
