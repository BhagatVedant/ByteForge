#include "storage.h"
#include "terminal.h"
#include "hello_file.h"
#include "notes_file.h"
#include "resume_file.h"

static int storage_mounted = 0;

typedef struct {
    const char *name;
    const unsigned char *data;
    unsigned int length;
    const char *type;
} file_entry_t;

static file_entry_t files[] = {
    {"hello.txt",  hello_txt,  sizeof(hello_txt),  "text"},
    {"notes.txt",  notes_txt,  sizeof(notes_txt),  "text"},
    {"resume.txt", resume_txt, sizeof(resume_txt), "text"},
};

static const int file_count = sizeof(files) / sizeof(files[0]);

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
    if (!strings_equal(device, "usb0")) {
        terminal_set_color(0x00FF6666);
        terminal_write("Device not found: ");
        terminal_set_color(0x00FFFFFF);
        terminal_write(device);
        terminal_write("\n");
        return;
    }

    if (storage_mounted) {
        terminal_set_color(0x00FFCC00);
        terminal_write("usb0 is already mounted.\n");
        return;
    }

    storage_mounted = 1;
    terminal_set_color(0x00AAFFAA);
    terminal_write("Mounted usb0 successfully.\n");
}

void storage_unmount(void) {
    if (!storage_mounted) {
        terminal_set_color(0x00FF6666);
        terminal_write("No storage mounted.\n");
        return;
    }

    storage_mounted = 0;
    terminal_set_color(0x00AAFFAA);
    terminal_write("Unmounted usb0 successfully.\n");
}

void storage_list_files(void) {
    if (!storage_mounted) {
        terminal_set_color(0x00FF6666);
        terminal_write("No storage mounted.\n");
        return;
    }

    terminal_set_color(0x00FFFFFF);
    terminal_write("Files:\n");

    for (int i = 0; i < file_count; i++) {
        terminal_write(files[i].name);
        terminal_write("  ");
        terminal_write(files[i].type);
        terminal_write("\n");
    }
}

void storage_open_file(const char *name) {
    if (!storage_mounted) {
        terminal_set_color(0x00FF6666);
        terminal_write("No storage mounted.\n");
        return;
    }

    for (int i = 0; i < file_count; i++) {
        if (strings_equal(name, files[i].name)) {
            terminal_set_color(0x00FFFFFF);
            terminal_write("Opening ");
            terminal_write(files[i].name);
            terminal_write("...\n");

            for (unsigned int j = 0; j < files[i].length; j++) {
                terminal_write_char((char)files[i].data[j]);
            }
            terminal_write("\n");
            return;
        }
    }

    terminal_set_color(0x00FF6666);
    terminal_write("File not found: ");
    terminal_set_color(0x00FFFFFF);
    terminal_write(name);
    terminal_write("\n");
}

void storage_cat_file(const char *name) {
    if (!storage_mounted) {
        terminal_set_color(0x00FF6666);
        terminal_write("No storage mounted.\n");
        return;
    }

    for (int i = 0; i < file_count; i++) {
        if (strings_equal(name, files[i].name)) {
            terminal_set_color(0x00FFFFFF);
            for (unsigned int j = 0; j < files[i].length; j++) {
                terminal_write_char((char)files[i].data[j]);
            }
            terminal_write("\n");
            return;
        }
    }

    terminal_set_color(0x00FF6666);
    terminal_write("File not found: ");
    terminal_set_color(0x00FFFFFF);
    terminal_write(name);
    terminal_write("\n");
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

    terminal_set_color(0x00FFFFFF);
    terminal_write("Available files: 3\n");
}