#include "framebuffer.h"
#include "terminal.h"

static void delay(volatile unsigned int count) {
    while (count--) {
    }
}

void kernel_main(void) {
    if (framebuffer_init()) {
        terminal_init();

        terminal_set_color(0x00FFCC00);
        terminal_write("ByteForge OS v0.1\n");

        terminal_set_color(0x00FFFFFF);
        terminal_write("Framebuffer initialized: OK\n");

        terminal_set_color(0x00AAFFAA);
        terminal_write("System ready.\n\n");

        terminal_set_color(0x00FFFFFF);
        terminal_write("ByteForge > ");
    }

    while (1) {
        terminal_draw_cursor();
        delay(5000000);

        terminal_erase_cursor();
        delay(5000000);
    }
}