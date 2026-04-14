#include "framebuffer.h"
#include "terminal.h"
#include "shell.h"
#include "input.h"

static void delay(volatile unsigned int count) {
    while (count--) {
    }
}

static void simulate_typing(const char *text) {
    while (*text) {
        input_add_char(*text);
        delay(1200000);
        text++;
    }
    delay(2000000);
    input_submit();
    delay(4000000);
}

void kernel_main(void) {
    if (framebuffer_init()) {
        terminal_init();
        shell_init();
        shell_prompt();
        input_init();

        simulate_typing("help");
        simulate_typing("version");
        simulate_typing("echo ByteForge is alive");
        simulate_typing("klippy");
        simulate_typing("info");
    }

    while (1) {
        terminal_draw_cursor();
        delay(2500000);
        terminal_erase_cursor();
        delay(2500000);
    }
}