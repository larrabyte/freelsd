use crate::ports::UnsafePort;
use spin::{Lazy, Mutex};

/// Prints to COM1.
#[macro_export]
macro_rules! serial {
    ($($arg:tt)*) => ($crate::uart::COM1.lock().format(format_args!($($arg)*)));
}

/// Prints to COM1 with a newline.
#[macro_export]
macro_rules! serialln {
    () => ($crate::serial!("\n"));
    ($($arg:tt)*) => ($crate::serial!("{}\n", format_args!($($arg)*)));
}

pub static COM1: Lazy<Mutex<Uart>> = Lazy::new(|| {
    // On x86_64 platforms, the first serial interface is located at 0x3f8.
    unsafe {
        Mutex::new(Uart::new(0x3f8))
    }
});

/// A representation of the 16650 UART.
#[derive(Debug)]
pub struct Uart {
    transmitter_holding: UnsafePort<u8>,
    divisor_latch_low: UnsafePort<u8>,
    interrupt_enable: UnsafePort<u8>,
    divisor_latch_high: UnsafePort<u8>,
    fifo_control: UnsafePort<u8>,
    line_control: UnsafePort<u8>,
    modem_control: UnsafePort<u8>,
    line_status: UnsafePort<u8>,
}

impl Uart {
    /// Creates a new UART instance.
    ///
    /// # Safety
    /// Callers must ensure that the given I/O port is an uninitialised serial interface.
    unsafe fn new(port: u16) -> Self {
        let mut uart = Self {
            transmitter_holding: UnsafePort::new(port),
            divisor_latch_low: UnsafePort::new(port),
            interrupt_enable: UnsafePort::new(port + 1),
            divisor_latch_high: UnsafePort::new(port + 1),
            fifo_control: UnsafePort::new(port + 2),
            line_control: UnsafePort::new(port + 3),
            modem_control: UnsafePort::new(port + 4),
            line_status: UnsafePort::new(port + 5)
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
        uart.fifo_control.write(0xc7);

        // 6. Enable the data terminal and signal a request to send.
        uart.modem_control.write(0x03);

        uart
    }

    /// Writes a byte out to the serial inteface.
    fn write(&mut self, byte: u8) {
        // Reading the line status register has no side effects.
        // Writing to the transmitter holding buffer will send this
        // character out to the serial interface.
        unsafe {
            while (self.line_status.read() & 0x20) == 0 {
                core::hint::spin_loop();
            }

            self.transmitter_holding.write(byte);
        }
    }

    // Implementation detail for the serial macro.
    #[doc(hidden)]
    pub fn format(&mut self, args: core::fmt::Arguments) {
        core::fmt::Write::write_fmt(self, args).unwrap();
    }
}

impl core::fmt::Write for Uart {
    fn write_str(&mut self, s: &str) -> core::fmt::Result {
        for byte in s.bytes() {
            self.write(byte);
        }

        Ok(())
    }
}
