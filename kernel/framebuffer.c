/*
    File: framebuffer.c

    This file provides supports to draw directly on the screen.
    The Raspberry Pi firmware give us a framebuffer through the mailbox interface.
    After establishing that the OS is free to draw manually on the screen.
*/

#include "framebuffer.h"
#include "mailbox.h"
#include "font.h"
#include <stdint.h>

static uint32_t width;
static uint32_t height;
static uint32_t pitch;
static uint8_t *framebuffer;

//Mailbox defined in mailbox.c used to send properties to GPU to display
extern volatile uint32_t mailbox[36];

/*
    Initializes framebuffer by setting up mailbox messages to get specified properties as follows:
    - Set physical width and height to 1024x768
    - Set virtual width and height to 1024x768
    - virtual offset to (0, 0)
    - Set color depth to 32 bits per pixel
    - Request framebuffer allocation
    - Number of bytes in a row aka pitch
*/

int framebuffer_init(void) {
    //Total length of message
    mailbox[0] = 35 * 4;
    //Request code
    mailbox[1] = 0;

    //Set physical width and height
    mailbox[2]  = 0x00048003;
    mailbox[3]  = 8;
    mailbox[4]  = 8;
    mailbox[5]  = 1024;
    mailbox[6]  = 768;

    //Set virtual width and height
    mailbox[7]  = 0x00048004;
    mailbox[8]  = 8;
    mailbox[9]  = 8;
    mailbox[10] = 1024;
    mailbox[11] = 768;

    //Set virtual offset to (0, 0)
    mailbox[12] = 0x00048009;
    mailbox[13] = 8;
    mailbox[14] = 8;
    mailbox[15] = 0;
    mailbox[16] = 0;

    //Set color depth to 32 bits per pixel
    mailbox[17] = 0x00048005;
    mailbox[18] = 4;
    mailbox[19] = 4;
    mailbox[20] = 32;

    //Request framebuffer allocation
    mailbox[21] = 0x00040001;
    mailbox[22] = 8;
    mailbox[23] = 8;
    mailbox[24] = 16;
    mailbox[25] = 0;

    //Asking GPU for pitch
    mailbox[26] = 0x00040008;
    mailbox[27] = 4;
    mailbox[28] = 4;
    mailbox[29] = 0;

    //End tag
    mailbox[30] = 0;
    mailbox[31] = 0;
    mailbox[32] = 0;
    mailbox[33] = 0;
    mailbox[34] = 0;

    //Send the message through channel 8 which is the property channel.
    if (!mailbox_call(8)) {
        return 0;
    }

    //GPU writes back in the same array and We save them for later use in drawing functions
    width = mailbox[5];
    height = mailbox[6];
    pitch = mailbox[29];
    framebuffer = (uint8_t*)((uintptr_t)(mailbox[24] & 0x3FFFFFFF));

    return framebuffer != 0;
}

//Draw one pixel at (x,y) with each pixel being 4 bytes.
void draw_pixel(int x, int y, uint32_t color) {
    if (!framebuffer) return;
    if (x < 0 || y < 0) return;
    if ((uint32_t)x >= width || (uint32_t)y >= height) return;

    uint32_t *pixel = (uint32_t*)(framebuffer + y * pitch + x * 4);
    *pixel = color;
}

//Drawing a filled rectangle by filling each pixel
void draw_rect(int x, int y, int rect_width, int rect_height, uint32_t color) {
    for (int row = y; row < y + rect_height; row++) {
        for (int col = x; col < x + rect_width; col++) {
            draw_pixel(col, row, color);
        }
    }
}

/*
    Drawing 8x8 characters by using bitmap in font.h
    Each character has an array of 1s and 0s
    We iterate through each one and if it is 1 we draw if not we skip.
*/
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

//Drawing a string by drawing each character one by one.
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