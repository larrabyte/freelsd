use self::types::InterruptDescriptorTable;

use spin::Lazy;

mod types;
mod asm;

static IDT: Lazy<InterruptDescriptorTable> = Lazy::new(InterruptDescriptorTable::default);

/// Load the IDTR with the kernel's interrupt descriptor table.
///
/// # Safety
/// The IDTR should only be loaded during kernel initialisation.
pub unsafe fn load() {
    IDT.load();
}
