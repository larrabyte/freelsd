use self::types::GlobalDescriptorTable;

mod types;

static mut GDT: GlobalDescriptorTable = GlobalDescriptorTable::new();

/// Loads the GDTR with the kernel's global descriptor table.
///
/// # Safety
/// The GDTR should only be loaded during kernel initialisation.
pub unsafe fn load() {
    GDT.load();
}

