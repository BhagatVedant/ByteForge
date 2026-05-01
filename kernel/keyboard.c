/*
    File: keyboard.c

    This file is a simple keyboard queue layer.

    Right now since there is not real USB input.
    Characters are pushed into the queue by keyboard_push_char() to simulate real keyboard input.

    The rest of the OS is seperated from keyboard input so that in the future when keyboard input is implemented this is the only file to be changed.
*/

#include "keyboard.h"
#include "input.h"

#define KEYBOARD_QUEUE_SIZE 512

static char keyboard_queue[KEYBOARD_QUEUE_SIZE];
static int queue_head = 0;
static int queue_tail = 0;

//Helper function to check if the queue is empty
static int queue_is_empty(void) {
    return queue_head == queue_tail;
}

//Helper function to check if the queue is full
static int queue_is_full(void) {
    return ((queue_tail + 1) % KEYBOARD_QUEUE_SIZE) == queue_head;
}

//Reset queue when OS starts
void keyboard_init(void) {
    queue_head = 0;
    queue_tail = 0;
}

//Simulate keyboard input by pushing characters into the queue
void keyboard_push_char(char c) {
    if (queue_is_full()) {
        return;
    }

    keyboard_queue[queue_tail] = c;
    queue_tail = (queue_tail + 1) % KEYBOARD_QUEUE_SIZE;
}

//Processing each character in the queue and sending it down the pipeline to input_process_char() in input.c
void keyboard_poll(void) {
    while (!queue_is_empty()) {
        char c = keyboard_queue[queue_head];
        queue_head = (queue_head + 1) % KEYBOARD_QUEUE_SIZE;
        input_process_char(c);
    }
}