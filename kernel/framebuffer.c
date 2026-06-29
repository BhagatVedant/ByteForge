#include "framebuffer.h"
#include "mailbox.h"
#include "font.h"
#include <stdint.h>

static uint32_t width;
static uint32_t height;
static uint32_t pitch;
static uint8_t *framebuffer;

extern volatile uint32_t mailbox[36];

int framebuffer_init(void) {
    mailbox[0] = 35 * 4;
    mailbox[1] = 0;

    // Physical width/height
    mailbox[2]  = 0x00048003;
    mailbox[3]  = 8;
    mailbox[4]  = 8;
    mailbox[5]  = 1920;
    mailbox[6]  = 1080;

    // Virtual width/height
    mailbox[7]  = 0x00048004;
    mailbox[8]  = 8;
    mailbox[9]  = 8;
    mailbox[10] = 1920;
    mailbox[11] = 1080;

    // Virtual offset
    mailbox[12] = 0x00048009;
    mailbox[13] = 8;
    mailbox[14] = 8;
    mailbox[15] = 0;
    mailbox[16] = 0;

    // Depth
    mailbox[17] = 0x00048005;
    mailbox[18] = 4;
    mailbox[19] = 4;
    mailbox[20] = 32;

    // Allocate framebuffer
    mailbox[21] = 0x00040001;
    mailbox[22] = 8;
    mailbox[23] = 8;
    mailbox[24] = 16;
    mailbox[25] = 0;

    // Get pitch
    mailbox[26] = 0x00040008;
    mailbox[27] = 4;
    mailbox[28] = 4;
    mailbox[29] = 0;

    mailbox[30] = 0;
    mailbox[31] = 0;
    mailbox[32] = 0;
    mailbox[33] = 0;
    mailbox[34] = 0;

    if (!mailbox_call(MBOX_CHANNEL_PROPERTY)) {
        return 0;
    }

    width = mailbox[5];
    height = mailbox[6];
    pitch = mailbox[29];

    // Firmware returns a bus-style framebuffer address. For this simple low-memory
    // setup, masking like the old code still works for the framebuffer pointer.
    framebuffer = (uint8_t*)((uintptr_t)(mailbox[24] & 0x3FFFFFFF));

    return framebuffer != 0 && pitch != 0;
}

uint32_t framebuffer_get_width(void) {
    return width;
}

uint32_t framebuffer_get_height(void) {
    return height;
}

uint32_t framebuffer_get_pitch(void) {
    return pitch;
}

void draw_pixel(int x, int y, uint32_t color) {
    if (!framebuffer) return;
    if (x < 0 || y < 0) return;
    if ((uint32_t)x >= width || (uint32_t)y >= height) return;

    uint32_t *pixel = (uint32_t*)(framebuffer + y * pitch + x * 4);
    *pixel = color;
}

void draw_rect(int x, int y, int rect_width, int rect_height, uint32_t color) {
    for (int row = y; row < y + rect_height; row++) {
        for (int col = x; col < x + rect_width; col++) {
            draw_pixel(col, row, color);
        }
    }
}

void draw_char(int x, int y, char c, uint32_t color) {
    const uint8_t *glyph = font[(unsigned char)c];

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (glyph[row] & (1 << (7 - col))) {
                draw_pixel(x + col, y + row, color);
            }
        }
    }
}

void draw_char_scaled(int x, int y, char c, uint32_t color, int scale) {
    const uint8_t *glyph = font[(unsigned char)c];

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (glyph[row] & (1 << (7 - col))) {
                draw_rect(
                    x + col * scale,
                    y + row * scale,
                    scale,
                    scale,
                    color
                );
            }
        }
    }
}

void draw_string(int x, int y, const char *str, uint32_t color) {
    int cursor_x = x;
    int cursor_y = y;

    while (*str) {
        if (*str == '\n') {
            cursor_x = x;
            cursor_y += 10;
        } else {
            draw_char(cursor_x, cursor_y, *str, color);
            cursor_x += 8;
        }
        str++;
    }
}
