#include <stdint.h>
#include "mailbox.h"

// Raspberry Pi 5 / BCM2712 mailbox as seen by the ARM cores.
// Old Pi Zero value was 0x2000B880. Do not use that on Pi 5.
#define VIDEOCORE_MBOX  0x107C013880UL

#define MBOX0_READ      ((volatile uint32_t*)(VIDEOCORE_MBOX + 0x00))
#define MBOX0_STATUS    ((volatile uint32_t*)(VIDEOCORE_MBOX + 0x18))
#define MBOX1_WRITE     ((volatile uint32_t*)(VIDEOCORE_MBOX + 0x20))
#define MBOX1_STATUS    ((volatile uint32_t*)(VIDEOCORE_MBOX + 0x38))

#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

volatile uint32_t mailbox[36] __attribute__((aligned(16)));

static inline void data_sync_barrier(void) {
    __asm__ volatile ("dsb sy" ::: "memory");
}

int mailbox_call(uint8_t channel) {
    uint32_t request = ((uint32_t)((uintptr_t)mailbox) & ~0xFU) | (channel & 0xFU);

    data_sync_barrier();

    while (*MBOX1_STATUS & MBOX_FULL) {
    }

    *MBOX1_WRITE = request;

    while (1) {
        while (*MBOX0_STATUS & MBOX_EMPTY) {
        }

        uint32_t response = *MBOX0_READ;
        if (response == request) {
            data_sync_barrier();
            return mailbox[1] == MBOX_RESPONSE;
        }
    }
}
