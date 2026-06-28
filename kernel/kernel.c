#include "framebuffer.h"
#include "terminal.h"
#include "shell.h"
#include "input.h"
#include "keyboard.h"
#include "storage.h"

static void run_boot_demo(void) {
    const char *demo =
        "status\n"
        "devices\n"
        "mount usb0\n"
        "ls\n"
        "info hello.txt\n"
        "open hello.txt\n"
        "open notes.txt\n"
        "open resume.txt\n"
        "cat resume.txt\n"
        "open fake.txt\n"
        "unmount\n"
        "ls\n";

    while (*demo) {
        keyboard_push_char(*demo);
        demo++;
    }
}

void kernel_main(void) {
    volatile unsigned int blink_counter = 0;

    if (framebuffer_init()) {
        terminal_init();
        shell_init();
        input_init();
        keyboard_init();
        storage_init();
        shell_prompt();

        // This is still fake keyboard input. Real USB keyboard support is a
        // separate driver, not just a Pi 5 port change.
        run_boot_demo();
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
