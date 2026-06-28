#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

int framebuffer_init(void);

uint32_t framebuffer_get_width(void);
uint32_t framebuffer_get_height(void);
uint32_t framebuffer_get_pitch(void);

void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int rect_width, int rect_height, uint32_t color);
void draw_char(int x, int y, char c, uint32_t color);
void draw_char_scaled(int x, int y, char c, uint32_t color, int scale);
void draw_string(int x, int y, const char *str, uint32_t color);

#endif