#include <stdint.h>
#include "framebuffer.h"
#include "mailbox.h"

static uint32_t width;
static uint32_t height;
static uint32_t pitch;
static uint8_t *framebuffer;

extern volatile uint32_t mailbox[36];

int framebuffer_init(void) {
    mailbox[0] = 35 * 4;
    mailbox[1] = 0;

    mailbox[2] = 0x00048003;
    mailbox[3] = 8;
    mailbox[4] = 8;
    mailbox[5] = 1024;
    mailbox[6] = 768;

    mailbox[7] = 0x00048004;
    mailbox[8] = 8;
    mailbox[9] = 8;
    mailbox[10] = 1024;
    mailbox[11] = 768;

    mailbox[12] = 0x00048009;
    mailbox[13] = 8;
    mailbox[14] = 8;
    mailbox[15] = 0;
    mailbox[16] = 0;

    mailbox[17] = 0x00048005;
    mailbox[18] = 4;
    mailbox[19] = 4;
    mailbox[20] = 32;

    mailbox[21] = 0x00040001;
    mailbox[22] = 8;
    mailbox[23] = 8;
    mailbox[24] = 16;
    mailbox[25] = 0;

    mailbox[26] = 0x00040008;
    mailbox[27] = 4;
    mailbox[28] = 4;
    mailbox[29] = 0;

    mailbox[30] = 0;
    mailbox[31] = 0;
    mailbox[32] = 0;
    mailbox[33] = 0;
    mailbox[34] = 0;

    if (!mailbox_call(8)) {
        return 0;
    }

    width = mailbox[5];
    height = mailbox[6];
    pitch = mailbox[29];
    framebuffer = (uint8_t*)((uintptr_t)(mailbox[24] & 0x3FFFFFFF));

    return framebuffer != 0;
}

void draw_pixel(int x, int y, uint32_t color) {
    uint32_t *pixel = (uint32_t*)(framebuffer + y * pitch + x * 4);
    *pixel = color;
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int row = y; row < y + height; row++) {
        for (int col = x; col < x + width; col++) {
            draw_pixel(col, row, color);
        }
    }
}