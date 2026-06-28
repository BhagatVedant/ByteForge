#include "terminal.h"
#include "framebuffer.h"

static int cursor_x = 0;
static int cursor_y = 0;
static uint32_t terminal_color = 0x00E6E6E6;

static int cursor_enabled = 0;
static int cursor_visible = 0;

#define FONT_SCALE    2
#define CHAR_WIDTH    (8 * FONT_SCALE)
#define CHAR_HEIGHT   (8 * FONT_SCALE)
#define LINE_SPACING  22

#define TERM_START_X  64
#define TERM_START_Y  64

#define BG_COLOR      0x00000000
#define TEXT_COLOR    0x00E6E6E6
#define DIM_COLOR     0x00666666
#define ACCENT_COLOR  0x0000FFAA
#define ERROR_COLOR   0x00FF5555

static uint32_t screen_width(void) {
    return framebuffer_get_width();
}

static uint32_t screen_height(void) {
    return framebuffer_get_height();
}

static void terminal_newline(void) {
    cursor_x = TERM_START_X;
    cursor_y += LINE_SPACING;

    if ((uint32_t)(cursor_y + CHAR_HEIGHT + TERM_START_Y) >= screen_height()) {
        terminal_clear(BG_COLOR);
    }
}

static void clear_char_cell(int x, int y) {
    draw_rect(x, y, CHAR_WIDTH, CHAR_HEIGHT, BG_COLOR);
}

static void draw_boot_header(void) {
    terminal_set_color(ACCENT_COLOR);
    terminal_write("ByteForge OS v0.1\n");

    terminal_set_color(DIM_COLOR);
    terminal_write("--------------------------------\n");

    terminal_set_color(TEXT_COLOR);
}

void terminal_init(void) {
    cursor_x = TERM_START_X;
    cursor_y = TERM_START_Y;
    terminal_color = TEXT_COLOR;
    cursor_enabled = 0;
    cursor_visible = 0;

    terminal_clear(BG_COLOR);
    draw_boot_header();
}

void terminal_clear(uint32_t color) {
    draw_rect(0, 0, screen_width(), screen_height(), color);

    cursor_x = TERM_START_X;
    cursor_y = TERM_START_Y;
    cursor_visible = 0;
}

void terminal_set_color(uint32_t color) {
    terminal_color = color;
}

void terminal_write_char(char c) {
    if (cursor_visible) {
        terminal_erase_cursor();
    }

    if (c == '\n') {
        terminal_newline();
        return;
    }

    draw_char_scaled(cursor_x, cursor_y, c, terminal_color, FONT_SCALE);
    cursor_x += CHAR_WIDTH;

    if ((uint32_t)(cursor_x + CHAR_WIDTH + TERM_START_X) >= screen_width()) {
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
    if (!cursor_enabled || cursor_visible) {
        return;
    }

    draw_char_scaled(cursor_x, cursor_y, '_', terminal_color, FONT_SCALE);
    cursor_visible = 1;
}

void terminal_erase_cursor(void) {
    if (!cursor_visible) {
        return;
    }

    clear_char_cell(cursor_x, cursor_y);
    cursor_visible = 0;
}

void terminal_backspace(void) {
    if (cursor_visible) {
        terminal_erase_cursor();
    }

    if (cursor_x > TERM_START_X) {
        cursor_x -= CHAR_WIDTH;
        clear_char_cell(cursor_x, cursor_y);
    }
}

void terminal_enable_cursor(void) {
    cursor_enabled = 1;
    cursor_visible = 0;
    terminal_draw_cursor();
}

void terminal_disable_cursor(void) {
    terminal_erase_cursor();
    cursor_enabled = 0;
}

void terminal_toggle_cursor(void) {
    if (!cursor_enabled) {
        return;
    }

    if (cursor_visible) {
        terminal_erase_cursor();
    } else {
        terminal_draw_cursor();
    }
}