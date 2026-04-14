#include "terminal.h"
#include "framebuffer.h"

static int cursor_x = 0;
static int cursor_y = 0;
static uint32_t terminal_color = 0x00FFFFFF;

#define TERM_START_X  40
#define TERM_START_Y  40
#define CHAR_WIDTH    8
#define CHAR_HEIGHT   8
#define LINE_SPACING  10
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768
#define BG_COLOR      0x00101010

static void terminal_newline(void) {
    cursor_x = TERM_START_X;
    cursor_y += LINE_SPACING;

    if (cursor_y + CHAR_HEIGHT >= SCREEN_HEIGHT) {
        terminal_clear(BG_COLOR);
    }
}

static void clear_char_cell(int x, int y) {
    draw_rect(x, y, CHAR_WIDTH, CHAR_HEIGHT, BG_COLOR);
}

void terminal_init(void) {
    cursor_x = TERM_START_X;
    cursor_y = TERM_START_Y;
    terminal_color = 0x00FFFFFF;
    terminal_clear(BG_COLOR);
}

void terminal_clear(uint32_t color) {
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
    cursor_x = TERM_START_X;
    cursor_y = TERM_START_Y;
}

void terminal_set_color(uint32_t color) {
    terminal_color = color;
}

void terminal_write_char(char c) {
    if (c == '\n') {
        terminal_newline();
        return;
    }

    draw_char(cursor_x, cursor_y, c, terminal_color);
    cursor_x += CHAR_WIDTH;

    if (cursor_x + CHAR_WIDTH >= SCREEN_WIDTH) {
        terminal_newline();
    }
}

void terminal_write(const char *str) {
    while (*str) {
        terminal_write_char(*str);
        str++;
    }
}

void terminal_draw_cursor(void) {
    draw_char(cursor_x, cursor_y, '_', terminal_color);
}

void terminal_erase_cursor(void) {
    clear_char_cell(cursor_x, cursor_y);
}

void terminal_backspace(void) {
    if (cursor_x > TERM_START_X) {
        cursor_x -= CHAR_WIDTH;
        clear_char_cell(cursor_x, cursor_y);
    }
}