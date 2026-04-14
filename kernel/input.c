#include "input.h"
#include "terminal.h"
#include "shell.h"

#define INPUT_BUFFER_SIZE 128

static char input_buffer[INPUT_BUFFER_SIZE];
static int input_length = 0;

void input_init(void) {
    input_length = 0;
    input_buffer[0] = '\0';
}

void input_add_char(char c) {
    if (input_length >= INPUT_BUFFER_SIZE - 1) {
        return;
    }

    terminal_erase_cursor();

    input_buffer[input_length] = c;
    input_length++;
    input_buffer[input_length] = '\0';

    terminal_write_char(c);
    terminal_draw_cursor();
}

void input_backspace(void) {
    if (input_length <= 0) {
        return;
    }

    terminal_erase_cursor();

    input_length--;
    input_buffer[input_length] = '\0';

    terminal_backspace();
    terminal_draw_cursor();
}

void input_submit(void) {
    terminal_erase_cursor();
    terminal_write("\n");

    shell_execute(input_buffer);

    input_length = 0;
    input_buffer[0] = '\0';
}