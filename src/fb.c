#include "fb.h"
#include "font8x8.h"

// A tiny text console on top of the framebuffer Limine gives us.
// The framebuffer is just memory: writing a 32-bit value at
// (y * pitch + x * 4) colors the pixel at (x, y).

static struct limine_framebuffer *screen;
static uint32_t fg_color, bg_color;
static uint32_t scale = 1;
static uint64_t cursor_x, cursor_y; // In pixels

static void put_pixel(uint64_t x, uint64_t y, uint32_t color) {
    if (x >= screen->width || y >= screen->height) {
        return;
    }
    uint8_t *row = (uint8_t *)screen->address + y * screen->pitch;
    ((volatile uint32_t *)row)[x] = color;
}

void fb_init(struct limine_framebuffer *fb, uint32_t fg, uint32_t bg) {
    screen = fb;
    fg_color = fg;
    bg_color = bg;
    fb_clear();
}

void fb_clear(void) {
    for (uint64_t y = 0; y < screen->height; y++) {
        for (uint64_t x = 0; x < screen->width; x++) {
            put_pixel(x, y, bg_color);
        }
    }
    cursor_x = cursor_y = 0;
}

void fb_set_scale(uint32_t s) {
    scale = s ? s : 1;
}

void fb_set_color(uint32_t fg) {
    fg_color = fg;
}

static void newline(void) {
    cursor_x = 0;
    cursor_y += 8 * scale + 2 * scale;
    if (cursor_y + 8 * scale > screen->height) {
        // No scrolling yet: wrap back to the top.
        fb_clear();
    }
}

void fb_putc(char c) {
    if (screen == 0) {
        return;
    }
    if (c == '\n') {
        newline();
        return;
    }
    if (cursor_x + 8 * scale > screen->width) {
        newline();
    }

    const unsigned char *glyph = font8x8_basic[(unsigned char)c & 0x7F];
    for (uint32_t row = 0; row < 8; row++) {
        for (uint32_t col = 0; col < 8; col++) {
            uint32_t color = (glyph[row] >> col) & 1 ? fg_color : bg_color;
            // Draw each font pixel as a scale x scale block.
            for (uint32_t dy = 0; dy < scale; dy++) {
                for (uint32_t dx = 0; dx < scale; dx++) {
                    put_pixel(cursor_x + col * scale + dx,
                              cursor_y + row * scale + dy, color);
                }
            }
        }
    }
    cursor_x += 8 * scale;
}

void fb_write(const char *s) {
    for (; *s; s++) {
        fb_putc(*s);
    }
}
