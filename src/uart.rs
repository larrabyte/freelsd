#![allow(dead_code)]

use crate::ports::UnsafePort;

use core::{fmt, hint::spin_loop};
use spin::{Mutex, Lazy};

/// Prints to COM1.
#[macro_export]
macro_rules! serial {
    ($($arg:tt)*) => ($crate::uart::COM1.lock().format(format_args!($($arg)*)));
}

/// Prints to COM1, with a newline.
#[macro_export]
macro_rules! serialln {
    () => ($crate::print!("\n"));
    ($($arg:tt)*) => ($crate::serial!("{}\n", format_args!($($arg)*)));
}

// At least on x86_64, this should always be 0x3F8.
pub static COM1: Lazy<Mutex<Uart>> = Lazy::new(|| {
    let mut uart = unsafe {Uart::new(0x3F8)};
    uart.initialise();
    Mutex::new(uart)
});

/// A representation of the 16650 UART.
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
    /// Callers must ensure that the given port really does point to a serial interface and
    /// are responsible for ensuring that the interface is initialised before use.
    const unsafe fn new(port: u16) -> Self {
        Self {
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
        }
    }

    /// Initialises the serial interface.
    fn initialise(&mut self) {
        // This is fine if these ports really are apart of the serial interface.
        unsafe {
            // 1. Disable serial interrupts.
            self.interrupt_enable.write(0x00);

            // 2. Enable the DLAB in order to set the baud rate.
            self.line_control.write(0x80);

            // 3. Set the baud rate to 115,200bps.
            self.divisor_latch_low.write(0x01);
            self.divisor_latch_high.write(0x00);

            // 4. Disable the DLAB and configure the serial port
            // for 8-bit words, with no parity bits and 1 stop bit.
            self.line_control.write(0x03);

            // 5. Enable and clear FIFO buffers and set the interrupt trigger at 14 bytes.
            self.fifo_control.write(0xC7);

            // 6. Enable the data terminal and signal a request to send.
            self.modem_control.write(0x03);
        }
    }

    /// Writes a byte out to the serial inteface.
    fn write(&mut self, byte: u8) {
        // Reading the line status register has no side effects.
        // Writing to the transmitter holding buffer will send this
        // character through the serial interface, which is intended.
        unsafe {
            while (self.line_status.read() & 0x20) == 0 {
                spin_loop();
            }

            self.transmitter_holding.write(byte);
        }
    }

    /// Implementation detail for the serial macro.
    #[doc(hidden)]
    pub fn format(&mut self, args: fmt::Arguments) {
        fmt::Write::write_fmt(self, args).unwrap();
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
