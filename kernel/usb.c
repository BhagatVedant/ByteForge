#include "usb.h"
#include "terminal.h"

static int usb_initialized = 0;

void usb_init(void) {
    usb_initialized = 1;

    terminal_set_color(0x00FFAA00);
    terminal_write("USB: driver scaffold loaded.\n");
    terminal_write("USB: xHCI/RP1 driver not implemented yet.\n");
    terminal_set_color(0x00E6E6E6);
}

void usb_poll(void) {
    if (!usb_initialized) {
        return;
    }

    /*
        Future work:
        1. Find/init Pi 5 USB host controller.
        2. Reset ports.
        3. Enumerate connected USB devices.
        4. Find HID keyboard interface.
        5. Read keyboard interrupt reports.
        6. Send decoded chars to input_push_char().
    */
}