#include "storage.h"
#include "terminal.h"

static int storage_mounted = 0;

static const char *device_name = "usb0";
static const char *file_name = "hello.txt";
static const char *file_contents = "hello from my phone";

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

void storage_init(void) {
    storage_mounted = 0;
}

void storage_devices(void) {
    terminal_set_color(0x00FFFFFF);
    terminal_write("Detected devices:\n");
    terminal_write("usb0 - external storage device\n");
}

void storage_mount(const char *device) {
    if (strings_equal(device, device_name)) {
        storage_mounted = 1;
        terminal_set_color(0x00AAFFAA);
        terminal_write("Mounted usb0 successfully.\n");
    } else {
        terminal_set_color(0x00FF6666);
        terminal_write("Device not found: ");
        terminal_set_color(0x00FFFFFF);
        terminal_write(device);
        terminal_write("\n");
    }
}

void storage_list_files(void) {
    if (!storage_mounted) {
        terminal_set_color(0x00FF6666);
        terminal_write("No storage mounted.\n");
        return;
    }

    terminal_set_color(0x00FFFFFF);
    terminal_write("Files:\n");
    terminal_write("hello.txt\n");
}

void storage_open_file(const char *name) {
    if (!storage_mounted) {
        terminal_set_color(0x00FF6666);
        terminal_write("No storage mounted.\n");
        return;
    }

    if (strings_equal(name, file_name)) {
        terminal_set_color(0x00FFFFFF);
        terminal_write("Opening hello.txt...\n");
        terminal_write(file_contents);
        terminal_write("\n");
    } else {
        terminal_set_color(0x00FF6666);
        terminal_write("File not found: ");
        terminal_set_color(0x00FFFFFF);
        terminal_write(name);
        terminal_write("\n");
    }
}

void storage_status(void) {
    terminal_set_color(0x00FFFFFF);
    terminal_write("Storage service: active\n");

    if (storage_mounted) {
        terminal_set_color(0x00AAFFAA);
        terminal_write("Mounted device: usb0\n");
    } else {
        terminal_set_color(0x00FF6666);
        terminal_write("Mounted device: none\n");
    }
}