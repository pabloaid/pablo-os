#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>

bool serial_init(void);
void serial_putc(char c);
void serial_write(const char *s);

#endif
