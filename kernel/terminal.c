/*
    File: terminal.c

    This file builds a simple terminal using the framebuffer as a base.

    The terminal adds cursor support, colors, new lines, screen clearing and backspace.
*/

#include "terminal.h"
#include "framebuffer.h"

static int cursor_x = 0;
static int cursor_y = 0;
static uint32_t terminal_color = 0x00FFFFFF;

static int cursor_enabled = 0;
static int cursor_visible = 0;

#define TERM_START_X  40
#define TERM_START_Y  40
#define CHAR_WIDTH    8
#define CHAR_HEIGHT   8
#define LINE_SPACING  10
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768
#define BG_COLOR      0x00101010

//Helper function to move to a new line and scroll if necessary
static void terminal_newline(void) {
    cursor_x = TERM_START_X;
    cursor_y += LINE_SPACING;

    if (cursor_y + CHAR_HEIGHT >= SCREEN_HEIGHT) {
        terminal_clear(BG_COLOR);
    }
}

//Helper function to clear a character cell at (x,y) by drawing a filled rectangle of background color on top
static void clear_char_cell(int x, int y) {
    draw_rect(x, y, CHAR_WIDTH, CHAR_HEIGHT, BG_COLOR);
}

//Initialize the terminal state and clean the screen by drawing a filled rectangle of background color on top
void terminal_init(void) {
    cursor_x = TERM_START_X;
    cursor_y = TERM_START_Y;
    terminal_color = 0x00FFFFFF;
    cursor_enabled = 0;
    cursor_visible = 0;
    terminal_clear(BG_COLOR);
}

//Fill the screen with a color and reset cursor position
void terminal_clear(uint32_t color) {
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
    cursor_x = TERM_START_X;
    cursor_y = TERM_START_Y;
    cursor_visible = 0;
}

//Set the color for future text output
void terminal_set_color(uint32_t color) {
    terminal_color = color;
}

//Write a character at the cursor position. If the cursor is visible erase it first to prevent overlapping text on top of the cursor.
void terminal_write_char(char c) {
    if (cursor_visible) {
        terminal_erase_cursor();
    }

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

//Write a full string
void terminal_write(const char *str) {
    while (*str) {
        terminal_write_char(*str);
        str++;
    }
}

//Drawing the cursor (currently an underscore symbol) at the current cursor position
void terminal_draw_cursor(void) {
    if (!cursor_enabled || cursor_visible) {
        return;
    }

    draw_char(cursor_x, cursor_y, '_', terminal_color);
    cursor_visible = 1;
}

//Erase the cursor by drawing on top
void terminal_erase_cursor(void) {
    if (!cursor_visible) {
        return;
    }

    clear_char_cell(cursor_x, cursor_y);
    cursor_visible = 0;
}

//Move the cursor back and erase the character there
void terminal_backspace(void) {
    if (cursor_visible) {
        terminal_erase_cursor();
    }

    if (cursor_x > TERM_START_X) {
        cursor_x -= CHAR_WIDTH;
        clear_char_cell(cursor_x, cursor_y);
    }
}

//Turing on the cursor
void terminal_enable_cursor(void) {
    cursor_enabled = 1;
    cursor_visible = 0;
    terminal_draw_cursor();
}

//Disabling the cursor
void terminal_disable_cursor(void) {
    terminal_erase_cursor();
    cursor_enabled = 0;
}

//Creating a blinking cursor by drawing and erasing it repeatedly
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