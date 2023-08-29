use self::types::InterruptDescriptorTable;
use spin::Lazy;

mod types;
mod asm;

static IDT: Lazy<InterruptDescriptorTable> = Lazy::new(InterruptDescriptorTable::default);

/// Load the IDTR with the kernel's interrupt descriptor table.
pub fn load() {
    unsafe {
        IDT.load();
    }
}
