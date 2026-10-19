#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include "io.h"
#include "serial.h"
#include "fb.h"

// --- Limine requests -------------------------------------------------------
// The kernel tells Limine what it wants by placing these structures in a
// special section. Limine finds them before jumping to kmain and fills in
// the `response` pointers.

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_bootloader_info_request bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// --- Helpers ---------------------------------------------------------------

static void kprint(const char *s) {
    serial_write(s);
    fb_write(s);
}

static void kprint_dec(uint64_t n) {
    char buf[21];
    size_t i = sizeof(buf) - 1;
    buf[i] = '\0';
    do {
        buf[--i] = (char)('0' + n % 10);
        n /= 10;
    } while (n != 0);
    kprint(&buf[i]);
}

// --- Entry point -----------------------------------------------------------
// Limine jumps here in 64-bit long mode, with paging on and the kernel
// mapped at 0xffffffff80000000 (see linker.ld).

void kmain(void) {
    serial_init();
    serial_write("\n[pablo-os] kernel entered\n");

    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        serial_write("[pablo-os] bootloader does not support base revision 3\n");
        halt_forever();
    }

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        serial_write("[pablo-os] no framebuffer\n");
        halt_forever();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    fb_init(fb, 0xE0E0E0, 0x101828);

    fb_set_scale(3);
    fb_set_color(0x7FDBFF);
    kprint("Hello from Pablo OS! I hope you are doing well.\n");

    fb_set_scale(2);
    fb_set_color(0xE0E0E0);
    kprint("\n");

    if (bootloader_info_request.response != NULL) {
        kprint("Booted by ");
        kprint(bootloader_info_request.response->name);
        kprint(" ");
        kprint(bootloader_info_request.response->version);
        kprint("\n");
    }

    kprint("Screen: ");
    kprint_dec(fb->width);
    kprint("x");
    kprint_dec(fb->height);
    kprint(" @ ");
    kprint_dec(fb->bpp);
    kprint(" bpp\n");

    kprint("\nNothing else to do yet. Halting.\n");
    halt_forever();
}
