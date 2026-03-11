#include <stdint.h>
#include "mailbox.h"

#define VIDEOCORE_MBOX  0x2000B880

#define MBOX_READ       ((volatile uint32_t*)(VIDEOCORE_MBOX + 0x0))
#define MBOX_STATUS     ((volatile uint32_t*)(VIDEOCORE_MBOX + 0x18))
#define MBOX_WRITE      ((volatile uint32_t*)(VIDEOCORE_MBOX + 0x20))

#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

volatile uint32_t mailbox[36] __attribute__((aligned(16)));

int mailbox_call(uint8_t channel) {
    uint32_t addr = ((uint32_t)((uintptr_t)mailbox) & ~0xF) | (channel & 0xF);

    while (*MBOX_STATUS & MBOX_FULL) {
    }

    *MBOX_WRITE = addr;

    while (1) {
        while (*MBOX_STATUS & MBOX_EMPTY) {
        }

        uint32_t response = *MBOX_READ;
        if (response == addr) {
            return mailbox[1] == MBOX_RESPONSE;
        }
    }
}