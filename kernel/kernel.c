#include "framebuffer.h"
#include "terminal.h"
#include "shell.h"
#include "input.h"
#include "keyboard.h"
#include "storage.h"
#include "usb.h"

void kernel_main(void) {
    volatile unsigned int blink_counter = 0;

    if (framebuffer_init()) {
        terminal_init();
        shell_init();
        input_init();
        keyboard_init();
        storage_init();
        shell_prompt();
        usb_init();
        usb_test_descriptor();

        keyboard_push_char('s');
        keyboard_push_char('t');
        keyboard_push_char('a');
        keyboard_push_char('t');
        keyboard_push_char('u');
        keyboard_push_char('s');
        keyboard_push_char('\n');

        keyboard_push_char('d');
        keyboard_push_char('e');
        keyboard_push_char('v');
        keyboard_push_char('i');
        keyboard_push_char('c');
        keyboard_push_char('e');
        keyboard_push_char('s');
        keyboard_push_char('\n');

        keyboard_push_char('m');
        keyboard_push_char('o');
        keyboard_push_char('u');
        keyboard_push_char('n');
        keyboard_push_char('t');
        keyboard_push_char(' ');
        keyboard_push_char('u');
        keyboard_push_char('s');
        keyboard_push_char('b');
        keyboard_push_char('0');
        keyboard_push_char('\n');

        keyboard_push_char('l');
        keyboard_push_char('s');
        keyboard_push_char('\n');

        keyboard_push_char('i');
        keyboard_push_char('n');
        keyboard_push_char('f');
        keyboard_push_char('o');
        keyboard_push_char(' ');
        keyboard_push_char('h');
        keyboard_push_char('e');
        keyboard_push_char('l');
        keyboard_push_char('l');
        keyboard_push_char('o');
        keyboard_push_char('.');
        keyboard_push_char('t');
        keyboard_push_char('x');
        keyboard_push_char('t');
        keyboard_push_char('\n');

        keyboard_push_char('o');
        keyboard_push_char('p');
        keyboard_push_char('e');
        keyboard_push_char('n');
        keyboard_push_char(' ');
        keyboard_push_char('h');
        keyboard_push_char('e');
        keyboard_push_char('l');
        keyboard_push_char('l');
        keyboard_push_char('o');
        keyboard_push_char('.');
        keyboard_push_char('t');
        keyboard_push_char('x');
        keyboard_push_char('t');
        keyboard_push_char('\n');
    }

    while (1) {
        keyboard_poll();

        blink_counter++;
        if (blink_counter >= 50000) {
            terminal_toggle_cursor();
            blink_counter = 0;
        }
    }
}