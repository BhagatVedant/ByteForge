#ifndef HID_KEYBOARD_H
#define HID_KEYBOARD_H

#include <stdint.h>

void hid_keyboard_init(void);
void hid_keyboard_handle_report(const uint8_t *report, uint32_t length);

#endif