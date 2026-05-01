/*
    File: input.c

    This file handles typed input before it is processed by the shell.

    Characters are stored in a buffer while also being printed to the terminal.
    Enter submits the buffer and backspace pops the last character from the buffer and removes it from the terminal.

    This file is a pipeline between the keyboard and the shell where it basically converts l s enter into "ls\n" for easy processing by the shell.
*/

#include "input.h"
#include "terminal.h"
#include "shell.h"

#define INPUT_BUFFER_SIZE 128

static char input_buffer[INPUT_BUFFER_SIZE];
static int input_length = 0;

//Reseting buffer
void input_init(void) {
    input_length = 0;
    input_buffer[0] = '\0';
}

/*
    Add a character to the buffer and print it to the terminal.
    The buffer is null-terminated to be passed as a string to the shell later.
*/
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

//Remove the last character from the buffer and remove it from the terminal.
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

/*
    Submitting the buffer to the shell and resetting it for next input.
    Also printing a new line to the terminal to keep formatting clean.
*/
void input_submit(void) {
    terminal_erase_cursor();
    terminal_write("\n");

    shell_execute(input_buffer);

    input_length = 0;
    input_buffer[0] = '\0';
}

//Main input handler checks for special characters like enter and backspace and calls the appropriate functions.
void input_process_char(char c) {
    if (c == '\n' || c == '\r') {
        input_submit();
    }
    else if (c == '\b') {
        input_backspace();
    }
    else {
        input_add_char(c);
    }
}