#include "shell.h"
#include "terminal.h"

static int strings_equal(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b;
}

static int starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str != *prefix) {
            return 0;
        }
        str++;
        prefix++;
    }
    return 1;
}

void shell_init(void) {
    terminal_set_color(0x00FFCC00);
    terminal_write("ByteForge OS v0.1\n");

    terminal_set_color(0x00FFFFFF);
    terminal_write("Framebuffer initialized: OK\n");

    terminal_set_color(0x00AAFFAA);
    terminal_write("System ready.\n\n");
}

void shell_prompt(void) {
    terminal_set_color(0x00FFFFFF);
    terminal_write("ByteForge > ");
    terminal_draw_cursor();
}

void shell_execute(const char *command) {
    terminal_erase_cursor();

    if (strings_equal(command, "help")) {
        terminal_set_color(0x00FFFFFF);
        terminal_write("Commands:\n");
        terminal_write("help     - show available commands\n");
        terminal_write("about    - about ByteForge\n");
        terminal_write("info     - show system info\n");
        terminal_write("clear    - clear the screen\n");
        terminal_write("version  - show OS version\n");
        terminal_write("echo ... - print text\n");
        terminal_write("klippy   - ???\n");
    }
    else if (strings_equal(command, "about")) {
        terminal_set_color(0x00FFFFFF);
        terminal_write("ByteForge is a bare-metal OS project for Raspberry Pi Zero W.\n");
        terminal_write("Built from scratch in ARM assembly and C.\n");
    }
    else if (strings_equal(command, "info")) {
        terminal_set_color(0x00FFFFFF);
        terminal_write("System: Raspberry Pi Zero W\n");
        terminal_write("Display: Framebuffer active\n");
        terminal_write("Status: Running custom kernel\n");
    }
    else if (strings_equal(command, "version")) {
        terminal_set_color(0x00FFCC00);
        terminal_write("ByteForge OS v0.1\n");
    }
    else if (strings_equal(command, "clear")) {
        terminal_clear(0x00101010);
        shell_prompt();
        return;
    }
    else if (strings_equal(command, "klippy")) {
        terminal_set_color(0x00FF66CC);
        terminal_write("Klippy? That guy?\n");
        terminal_set_color(0x00FFFFFF);
        terminal_write("Yes, that's the one. That's him. The CS Goat.\n");
    }
    else if (starts_with(command, "echo ")) {
        terminal_set_color(0x00FFFFFF);
        terminal_write(command + 5);
        terminal_write("\n");
    }
    else if (strings_equal(command, "echo")) {
        terminal_write("\n");
    }
    else {
        terminal_set_color(0x00FF6666);
        terminal_write("Unknown command: ");
        terminal_set_color(0x00FFFFFF);
        terminal_write(command);
        terminal_write("\n");
    }

    terminal_write("\n");
    shell_prompt();
}