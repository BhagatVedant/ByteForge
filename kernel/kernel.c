#include "framebuffer.h"
#include "terminal.h"
#include "shell.h"
#include "input.h"
#include "storage.h"
#include "usb.h"
#include "hid_keyboard.h"

void kernel_main(void) {
    volatile unsigned int blink_counter = 0;

    if (!framebuffer_init()) {
        while (1) {
            // Framebuffer failed, nothing useful to draw yet.
        }
    }

    terminal_init();

    input_init();
    storage_init();
    shell_init();

    hid_keyboard_init();
    usb_init();

    terminal_set_color(0x00E6E6E6);
    terminal_write("\n");
    shell_prompt();

    while (1) {
        usb_poll();
        shell_update();

        blink_counter++;
        if (blink_counter >= 50000) {
            terminal_toggle_cursor();
            blink_counter = 0;
        }
    }
}