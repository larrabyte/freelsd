#![feature(panic_info_message)]
#![feature(abi_x86_interrupt)]
#![feature(custom_test_frameworks)]
#![test_runner(test::dispatch)]
#![reexport_test_harness_main = "testing"]
#![no_main]
#![no_std]

mod instructions;
mod memory;
mod ports;
mod panic;
mod boot;
mod uart;
mod idt;
mod gdt;

use crate::boot::BOOTLOADER_INFORMATION;

#[no_mangle]
extern "C" fn main() -> ! {
    serialln!("\n  ()-()\n.-(___)-. freelsd kernel\n _<   >_  you know the last plane out of sydney's almost gone?\n \\/   \\/\n");

    let bootloader = BOOTLOADER_INFORMATION.get_response().get().unwrap();
    serialln!("bootloader name: {:?}", bootloader.name.to_str().unwrap());
    serialln!("bootloader version: {:?}", bootloader.version.to_str().unwrap());

    gdt::load();
    idt::load();

    #[cfg(test)]
    testing();

    loop {
        instructions::hlt();
    }
}

#[cfg(test)]
mod test {
    use crate::{serial, serialln, ports::UnsafePort, instructions};

    pub trait Testable {
        fn run(&self);
    }

    pub fn dispatch(tests: &[&dyn Testable]) -> ! {
        serialln!("running {} tests", tests.len());

        for test in tests {
            test.run();
        }

        // This intentionally exits QEMU as long as a debug device was setup.
        unsafe {
            UnsafePort::new(0xF4).write(0u32);
        }

        loop {
            instructions::hlt();
        }
    }

    impl<T: Fn()> Testable for T {
        fn run(&self) {
            serial!("test {} ... ", core::any::type_name::<T>());
            self();
            serialln!("ok");
        }
    }
}
