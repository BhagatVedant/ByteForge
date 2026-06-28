#include "hid_keyboard.h"
#include "input.h"

static char keycode_to_ascii(uint8_t keycode, int shift_pressed) {
    if (keycode >= 0x04 && keycode <= 0x1D) {
        char c = 'a' + (keycode - 0x04);

        if (shift_pressed) {
            c = 'A' + (keycode - 0x04);
        }

        return c;
    }

    if (keycode >= 0x1E && keycode <= 0x26) {
        const char normal_digits[] = "123456789";
        const char shifted_digits[] = "!@#$%^&*(";

        return shift_pressed
            ? shifted_digits[keycode - 0x1E]
            : normal_digits[keycode - 0x1E];
    }

    if (keycode == 0x27) return shift_pressed ? ')' : '0';
    if (keycode == 0x28) return '\n';
    if (keycode == 0x2A) return '\b';
    if (keycode == 0x2C) return ' ';
    if (keycode == 0x2D) return shift_pressed ? '_' : '-';
    if (keycode == 0x2E) return shift_pressed ? '+' : '=';
    if (keycode == 0x33) return shift_pressed ? ':' : ';';
    if (keycode == 0x36) return shift_pressed ? '<' : ',';
    if (keycode == 0x37) return shift_pressed ? '>' : '.';
    if (keycode == 0x38) return shift_pressed ? '?' : '/';

    return 0;
}

void hid_keyboard_init(void) {
    /*
        HID keyboard parser is ready.
        It still needs real USB reports from usb.c.
    */
}

void hid_keyboard_handle_report(const uint8_t *report, uint32_t length) {
    if (!report || length < 8) {
        return;
    }

    uint8_t modifiers = report[0];
    int shift_pressed = (modifiers & 0x22) != 0;

    for (int i = 2; i < 8; i++) {
        uint8_t keycode = report[i];

        if (keycode == 0) {
            continue;
        }

        char c = keycode_to_ascii(keycode, shift_pressed);

        if (c) {
            input_push_char(c);
        }
    }
}