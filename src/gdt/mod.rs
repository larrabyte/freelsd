use self::types::GlobalDescriptorTable;

mod types;

static mut GDT: GlobalDescriptorTable = GlobalDescriptorTable::new();

/// Loads the GDTR with the kernel's global descriptor table.
pub fn load() {
    unsafe {
        GDT.load();
    }
}

