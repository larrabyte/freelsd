#![allow(dead_code)]

use crate::ports::UnsafePort;

use core::{fmt, arch::asm, hint::spin_loop};
use spin::{Mutex, Lazy};

// Prints to COM1.
#[macro_export]
macro_rules! serial {
    ($($arg:tt)*) => ($crate::uart::COM1.lock().format(format_args!($($arg)*)));
}

// Prints to COM1 with a newline.
#[macro_export]
macro_rules! serialln {
    () => ($crate::serial!("\n"));
    ($($arg:tt)*) => ($crate::serial!("{}\n", format_args!($($arg)*)));
}

// At least on x86_64, this should always be 0x3F8.
pub static COM1: Lazy<Mutex<Uart>> = Lazy::new(|| {
    Mutex::new(unsafe {Uart::new(0x3F8)})
});

/// A representation of the 16650 UART.
#[derive(Debug)]
pub struct Uart {
    transmitter_holding: UnsafePort<u8>,
    receiver: UnsafePort<u8>,
    divisor_latch_low: UnsafePort<u8>,
    interrupt_enable: UnsafePort<u8>,
    divisor_latch_high: UnsafePort<u8>,
    interrupt_identify: UnsafePort<u8>,
    fifo_control: UnsafePort<u8>,
    line_control: UnsafePort<u8>,
    modem_control: UnsafePort<u8>,
    line_status: UnsafePort<u8>,
    modem_status: UnsafePort<u8>,
    scratch: UnsafePort<u8>
}

impl Uart {
    /// Creates a new UART instance.
    ///
    /// # Safety
    /// Callers must ensure that the given I/O port is an uninitialised serial interface.
    unsafe fn new(port: u16) -> Self {
        let mut uart = Self {
            transmitter_holding: UnsafePort::new(port),
            receiver: UnsafePort::new(port),
            divisor_latch_low: UnsafePort::new(port),
            interrupt_enable: UnsafePort::new(port + 1),
            divisor_latch_high: UnsafePort::new(port + 1),
            interrupt_identify: UnsafePort::new(port + 2),
            fifo_control: UnsafePort::new(port + 2),
            line_control: UnsafePort::new(port + 3),
            modem_control: UnsafePort::new(port + 4),
            line_status: UnsafePort::new(port + 5),
            modem_status: UnsafePort::new(port + 6),
            scratch: UnsafePort::new(port + 7)
        };

        // 1. Disable serial interrupts.
        uart.interrupt_enable.write(0x00);

        // 2. Enable the DLAB in order to set the baud rate.
        uart.line_control.write(0x80);

        // 3. Set the baud rate to 115,200bps.
        uart.divisor_latch_low.write(0x01);
        uart.divisor_latch_high.write(0x00);

        // 4. Disable the DLAB and configure the serial port
        // for 8-bit words, with no parity bits and 1 stop bit.
        uart.line_control.write(0x03);

        // 5. Enable and clear FIFO buffers and set the interrupt trigger at 14 bytes.
        uart.fifo_control.write(0xC7);

        // 6. Enable the data terminal and signal a request to send.
        uart.modem_control.write(0x03);

        uart
    }

    /// Writes a byte out to the serial inteface.
    fn write(&mut self, byte: u8) {
        // SAFETY: Reading the line status register has no side effects.
        // Writing to the transmitter holding buffer will send this
        // character through the serial interface, which is intended.
        unsafe {
            while (self.line_status.read() & 0x20) == 0 {
                spin_loop();
            }

            self.transmitter_holding.write(byte);
        }
    }

    // Implementation detail for the serial macro.
    #[doc(hidden)]
    pub fn format(&mut self, args: fmt::Arguments) {
        unsafe {asm!("cli", options(nomem, preserves_flags))}
        fmt::Write::write_fmt(self, args).unwrap();
        unsafe {asm!("sti", options(nomem, preserves_flags))}
    }
}

impl fmt::Write for Uart {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        for byte in s.bytes() {
            self.write(byte);
        }

        Ok(())
    }
}
