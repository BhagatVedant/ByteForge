#include "shell.h"
#include "terminal.h"

typedef void (*command_handler_t)(const char *args);

typedef struct {
    const char *name;
    command_handler_t handler;
    const char *description;
} shell_command_t;

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

static void skip_spaces(const char **str) {
    while (**str == ' ') {
        (*str)++;
    }
}

static void cmd_help(const char *args);
static void cmd_about(const char *args);
static void cmd_info(const char *args);
static void cmd_clear(const char *args);
static void cmd_version(const char *args);
static void cmd_echo(const char *args);
static void cmd_klippy(const char *args);
static void cmd_credits(const char *args);

static shell_command_t commands[] = {
    {"help",    cmd_help,    "show available commands"},
    {"about",   cmd_about,   "about ByteForge"},
    {"info",    cmd_info,    "show system info"},
    {"clear",   cmd_clear,   "clear the screen"},
    {"version", cmd_version, "show OS version"},
    {"echo",    cmd_echo,    "print text"},
    {"klippy",  cmd_klippy,  "???"},
    {"credits", cmd_credits, "show credits"},
};

static const int command_count = sizeof(commands) / sizeof(commands[0]);

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

static void cmd_help(const char *args) {
    (void)args;
    terminal_set_color(0x00FFFFFF);
    terminal_write("Commands:\n");

    for (int i = 0; i < command_count; i++) {
        terminal_write(commands[i].name);

        if (commands[i].name[0] == 'e') {
            terminal_write(" ...");
        }

        if (commands[i].name[0] == 'h') terminal_write("    ");
        else if (commands[i].name[0] == 'i') terminal_write("    ");
        else if (commands[i].name[0] == 'c') terminal_write("   ");
        else if (commands[i].name[0] == 'k') terminal_write("  ");
        else terminal_write(" ");

        terminal_write("- ");
        terminal_write(commands[i].description);
        terminal_write("\n");
    }
}

static void cmd_about(const char *args) {
    (void)args;
    terminal_set_color(0x00FFFFFF);
    terminal_write("ByteForge is a bare-metal OS project for Raspberry Pi Zero W.\n");
    terminal_write("Built from scratch in ARM assembly and C.\n");
}

static void cmd_info(const char *args) {
    (void)args;
    terminal_set_color(0x00FFFFFF);
    terminal_write("System: Raspberry Pi Zero W\n");
    terminal_write("Display: Framebuffer active\n");
    terminal_write("Status: Running custom kernel\n");
}

static void cmd_clear(const char *args) {
    (void)args;
    terminal_clear(0x00101010);
}

static void cmd_version(const char *args) {
    (void)args;
    terminal_set_color(0x00FFCC00);
    terminal_write("ByteForge OS v0.1\n");
}

static void cmd_echo(const char *args) {
    terminal_set_color(0x00FFFFFF);
    terminal_write(args);
    terminal_write("\n");
}

static void cmd_klippy(const char *args) {
    (void)args;
    terminal_set_color(0x00FF66CC);
    terminal_write("Klippy? That guy?\n");
    terminal_set_color(0x00FFFFFF);
    terminal_write("Yes, that's the one. That's him. The CS Goat.\n");
}

static void cmd_credits(const char *args) {
    (void)args;
    terminal_set_color(0x00FFFFFF);
    terminal_write("ByteForge by Vedant Bhagat\n");
    terminal_write("Built on Raspberry Pi Zero W\n");
}

void shell_execute(const char *command) {
    terminal_erase_cursor();

    const char *args = command;
    while (*args && *args != ' ') {
        args++;
    }

    const char *arg_start = args;
    if (*arg_start == ' ') {
        arg_start++;
    }
    skip_spaces(&arg_start);

    for (int i = 0; i < command_count; i++) {
        const char *name = commands[i].name;
        int match = 1;

        int j = 0;
        while (name[j] && command[j] && command[j] != ' ') {
            if (name[j] != command[j]) {
                match = 0;
                break;
            }
            j++;
        }

        if (match && name[j] == '\0' && (command[j] == '\0' || command[j] == ' ')) {
            commands[i].handler(arg_start);

            if (!strings_equal(commands[i].name, "clear")) {
                terminal_write("\n");
            }

            shell_prompt();
            return;
        }
    }

    terminal_set_color(0x00FF6666);
    terminal_write("Unknown command: ");
    terminal_set_color(0x00FFFFFF);
    terminal_write(command);
    terminal_write("\n\n");
    shell_prompt();
}