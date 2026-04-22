#include "shell.h"
#include "terminal.h"
#include "storage.h"

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
static void cmd_sysinfo(const char *args);
static void cmd_clear(const char *args);
static void cmd_version(const char *args);
static void cmd_echo(const char *args);
static void cmd_klippy(const char *args);
static void cmd_credits(const char *args);
static void cmd_devices(const char *args);
static void cmd_mount(const char *args);
static void cmd_ls(const char *args);
static void cmd_open(const char *args);
static void cmd_status(const char *args);
static void cmd_cat(const char *args);
static void cmd_unmount(const char *args);
static void cmd_info(const char *args);

static shell_command_t commands[] = {
    {"help",    cmd_help,    "show available commands"},
    {"about",   cmd_about,   "about ByteForge"},
    {"sysinfo",    cmd_sysinfo,    "show system info"},
    {"status",  cmd_status,  "show storage status"},
    {"devices", cmd_devices, "list storage devices"},
    {"mount",   cmd_mount,   "mount a device"},
    {"ls",      cmd_ls,      "list files"},
    {"open",    cmd_open,    "open a file"},
    {"clear",   cmd_clear,   "clear the screen"},
    {"version", cmd_version, "show OS version"},
    {"echo",    cmd_echo,    "print text"},
    {"klippy",  cmd_klippy,  "???"},
    {"credits", cmd_credits, "show credits"},
    {"cat",     cmd_cat,     "print a file in terminal"},
    {"unmount", cmd_unmount, "unmount current device"},
    {"info", cmd_info,   "show file info"},
};

static const int command_count = sizeof(commands) / sizeof(commands[0]);

void shell_init(void) {
    terminal_set_color(0x00FFCC00);
    terminal_write("ByteForge Storage OS v0.1\n");

    terminal_set_color(0x00FFFFFF);
    terminal_write("Framebuffer initialized: OK\n");

    terminal_set_color(0x00AAFFAA);
    terminal_write("Storage service initialized: OK\n");
    terminal_write("Server ready.\n\n");
}

void shell_prompt(void) {
    terminal_set_color(0x00FFFFFF);
    terminal_write("storage@byteforge > ");
    terminal_enable_cursor();
}

static void cmd_help(const char *args) {
    (void)args;
    terminal_set_color(0x00FFFFFF);
    terminal_write("Commands:\n");

    for (int i = 0; i < command_count; i++) {
        terminal_write(commands[i].name);

        if (strings_equal(commands[i].name, "echo") ||
            strings_equal(commands[i].name, "mount") ||
            strings_equal(commands[i].name, "open")) {
            terminal_write(" ...");
        }

        terminal_write(" - ");
        terminal_write(commands[i].description);
        terminal_write("\n");
    }
}

static void cmd_about(const char *args) {
    (void)args;
    terminal_set_color(0x00FFFFFF);
    terminal_write("ByteForge is a bare-metal storage-oriented OS project\n");
    terminal_write("for Raspberry Pi Zero W, written in ARM assembly and C.\n");
}

static void cmd_sysinfo(const char *args) {
    (void)args;
    terminal_set_color(0x00FFFFFF);
    terminal_write("System: Raspberry Pi Zero W\n");
    terminal_write("Display: Framebuffer active\n");
    terminal_write("Shell: Online\n");
    terminal_write("Storage mode: Prototype\n");
}

static void cmd_status(const char *args) {
    (void)args;
    storage_status();
}

static void cmd_devices(const char *args) {
    (void)args;
    storage_devices();
}

static void cmd_mount(const char *args) {
    storage_mount(args);
}

static void cmd_ls(const char *args) {
    (void)args;
    storage_list_files();
}

static void cmd_open(const char *args) {
    storage_open_file(args);
}

static void cmd_clear(const char *args) {
    (void)args;
    terminal_clear(0x00101010);
}

static void cmd_version(const char *args) {
    (void)args;
    terminal_set_color(0x00FFCC00);
    terminal_write("ByteForge Storage OS v0.1\n");
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
    terminal_write("Yea bro. The one building a storage OS on a Pi for fun.\n");
}

static void cmd_credits(const char *args) {
    (void)args;
    terminal_set_color(0x00FFFFFF);
    terminal_write("ByteForge by Vedant Bhagat\n");
    terminal_write("Built on Raspberry Pi Zero W\n");
}

static void cmd_cat(const char *args) {
    storage_cat_file(args);
}

static void cmd_unmount(const char *args) {
    (void)args;
    storage_unmount();
}

static void cmd_info(const char *args) {
    storage_info_file(args);
}

void shell_execute(const char *command) {
    terminal_disable_cursor();

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