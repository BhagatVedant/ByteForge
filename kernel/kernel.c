#include "framebuffer.h"
#include "terminal.h"
#include "input.h"
#include "keyboard.h"
#include "usb.h"

void kernel_main(void) {
    volatile unsigned int blink_counter = 0;

    if (framebuffer_init()) {
        terminal_init();
        input_init();
        keyboard_init();

        terminal_set_color(0x00FFCC00);
        terminal_write("ByteForge USB Keyboard Test\n\n");

        terminal_set_color(0x00FFFFFF);
        usb_init();
        usb_enumerate();

        terminal_write("\nType something below:\n\n> ");
        terminal_enable_cursor();
    }

    while (1) {
        keyboard_poll();

        while (keyboard_has_char()) {
            char c = keyboard_get_char();
            input_process_char(c);
        }

        blink_counter++;
        if (blink_counter >= 50000) {
            terminal_toggle_cursor();
            blink_counter = 0;
        }
    }
}