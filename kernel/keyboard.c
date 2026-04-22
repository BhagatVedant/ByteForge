#include "keyboard.h"

#define KEYBOARD_QUEUE_SIZE 512

static char keyboard_queue[KEYBOARD_QUEUE_SIZE];
static int queue_head = 0;
static int queue_tail = 0;

static int queue_is_empty(void) {
    return queue_head == queue_tail;
}

static int queue_is_full(void) {
    return ((queue_tail + 1) % KEYBOARD_QUEUE_SIZE) == queue_head;
}

void keyboard_init(void) {
    queue_head = 0;
    queue_tail = 0;
}

void keyboard_push_char(char c) {
    if (queue_is_full()) {
        return;
    }

    keyboard_queue[queue_tail] = c;
    queue_tail = (queue_tail + 1) % KEYBOARD_QUEUE_SIZE;
}

int keyboard_has_char(void) {
    return !queue_is_empty();
}

char keyboard_get_char(void) {
    if (queue_is_empty()) {
        return 0;
    }

    char c = keyboard_queue[queue_head];
    queue_head = (queue_head + 1) % KEYBOARD_QUEUE_SIZE;
    return c;
}

void keyboard_poll(void) {
    // real USB keyboard logic goes here later
}