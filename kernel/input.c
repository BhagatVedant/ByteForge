#include "input.h"

#define INPUT_BUFFER_SIZE 128

static char input_buffer[INPUT_BUFFER_SIZE];
static int input_read_index = 0;
static int input_write_index = 0;

static int next_index(int index) {
    return (index + 1) % INPUT_BUFFER_SIZE;
}

void input_init(void) {
    input_read_index = 0;
    input_write_index = 0;
}

void input_push_char(char c) {
    int next = next_index(input_write_index);

    if (next == input_read_index) {
        return;
    }

    input_buffer[input_write_index] = c;
    input_write_index = next;
}

int input_has_char(void) {
    return input_read_index != input_write_index;
}

char input_get_char(void) {
    if (!input_has_char()) {
        return 0;
    }

    char c = input_buffer[input_read_index];
    input_read_index = next_index(input_read_index);

    return c;
}