#include "framebuffer.h"

void kernel_main(void) {
    if (framebuffer_init()) {
        draw_string(40, 40, "ByteForge OS v0.1", 0x00FFCC00);
        draw_string(40, 60, "Boot successful.", 0x00FFFFFF);
        draw_string(40, 80, "Raspberry Pi Zero W", 0x00AAAAFF);
    }

    while (1) {
    }
}