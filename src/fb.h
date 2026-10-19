#ifndef FB_H
#define FB_H

#include <stdint.h>
#include <limine.h>

void fb_init(struct limine_framebuffer *fb, uint32_t fg, uint32_t bg);
void fb_clear(void);
void fb_set_scale(uint32_t scale);
void fb_set_color(uint32_t fg);
void fb_putc(char c);
void fb_write(const char *s);

#endif
