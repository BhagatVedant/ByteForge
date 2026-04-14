#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

void terminal_init(void);
void terminal_clear(uint32_t color);
void terminal_set_color(uint32_t color);
void terminal_write_char(char c);
void terminal_write(const char *str);
void terminal_draw_cursor(void);
void terminal_erase_cursor(void);
void terminal_backspace(void);

#endif