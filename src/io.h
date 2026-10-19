#ifndef IO_H
#define IO_H

#include <stdint.h>

// x86 port I/O: the CPU talks to legacy devices (serial ports, keyboard
// controller, PIC) through a separate 16-bit "port" address space.

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Halt the CPU forever. Interrupts are disabled first so nothing wakes it.
static inline __attribute__((noreturn)) void halt_forever(void) {
    __asm__ volatile ("cli");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

#endif
