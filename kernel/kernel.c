#include "framebuffer.h"
#include "terminal.h"
#include "shell.h"

static void delay(volatile unsigned int count) {
    while (count--) {
    }
}

void kernel_main(void) {
    if (framebuffer_init()) {
        terminal_init();
        shell_init();
        shell_prompt();

        delay(8000000);
        shell_execute("help");

        delay(8000000);
        shell_execute("about");

        delay(8000000);
        shell_execute("info");
    }

    while (1) {
    }
}