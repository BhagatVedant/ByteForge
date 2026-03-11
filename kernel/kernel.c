#include "framebuffer.h"

void kernel_main(void) {
    if (framebuffer_init()) {
        draw_rect(0, 0, 1024, 768, 0x00101010);
        draw_rect(100, 100, 300, 120, 0x00FFCC00);
    }

    while (1) {
    }
}