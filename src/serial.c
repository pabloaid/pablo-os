#include "serial.h"
#include "io.h"

// COM1, the first serial port. QEMU forwards it to your terminal
// (see `-serial stdio` in the GNUmakefile), which makes it the easiest
// way to get debug output out of the kernel.
#define COM1 0x3F8

static bool serial_ok = false;

bool serial_init(void) {
    outb(COM1 + 1, 0x00); // Disable serial interrupts
    outb(COM1 + 3, 0x80); // Enable DLAB to set the baud rate divisor
    outb(COM1 + 0, 0x01); // Divisor low byte: 1 = 115200 baud
    outb(COM1 + 1, 0x00); // Divisor high byte
    outb(COM1 + 3, 0x03); // 8 data bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7); // Enable and clear FIFOs
    outb(COM1 + 4, 0x1E); // Loopback mode, to test the chip

    // Send a byte in loopback mode; if it comes back, the port works.
    outb(COM1 + 0, 0xAE);
    if (inb(COM1 + 0) != 0xAE) {
        return false; // No serial port (common on real laptops)
    }

    outb(COM1 + 4, 0x0F); // Back to normal operation
    serial_ok = true;
    return true;
}

void serial_putc(char c) {
    if (!serial_ok) {
        return;
    }
    // Wait until the transmit buffer is empty.
    while ((inb(COM1 + 5) & 0x20) == 0) {
    }
    outb(COM1, (uint8_t)c);
}

void serial_write(const char *s) {
    for (; *s; s++) {
        if (*s == '\n') {
            serial_putc('\r');
        }
        serial_putc(*s);
    }
}
