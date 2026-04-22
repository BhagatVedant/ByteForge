#include "usb.h"
#include "terminal.h"

#define USB_BASE 0x20980000

#define USB_GAHBCFG   ((volatile unsigned int*)(USB_BASE + 0x008))
#define USB_GUSBCFG   ((volatile unsigned int*)(USB_BASE + 0x00C))
#define USB_GRSTCTL   ((volatile unsigned int*)(USB_BASE + 0x010))
#define USB_HCFG      ((volatile unsigned int*)(USB_BASE + 0x400))
#define USB_HPRT      ((volatile unsigned int*)(USB_BASE + 0x440))

#define USB_HCCHAR0   ((volatile unsigned int*)(USB_BASE + 0x500))
#define USB_HCTSIZ0   ((volatile unsigned int*)(USB_BASE + 0x510))
#define USB_HCDMA0    ((volatile unsigned int*)(USB_BASE + 0x514))

static unsigned char setup_packet[8] __attribute__((aligned(4)));

static void delay(int count) {
    while (count--) {
        asm volatile("nop");
    }
}

void usb_init(void) {
    terminal_write("Initializing USB...\n");

    *USB_GRSTCTL |= 1;
    while (*USB_GRSTCTL & 1);

    delay(100000);

    *USB_GUSBCFG |= (1 << 29);
    *USB_GAHBCFG |= 1;
    *USB_HCFG |= 3;

    delay(100000);

    *USB_HPRT |= (1 << 12);
    delay(200000);

    *USB_HPRT |= (1 << 8);
    delay(200000);
    *USB_HPRT &= ~(1 << 8);
    delay(200000);

    if (*USB_HPRT & 1) {
        terminal_write("USB device detected.\n");
    } else {
        terminal_write("No USB device.\n");
    }
}

void usb_enumerate(void) {
    terminal_write("Sending REAL setup packet...\n");

    // --- GET_DESCRIPTOR DEVICE ---
    setup_packet[0] = 0x80; // bmRequestType
    setup_packet[1] = 6;    // GET_DESCRIPTOR
    setup_packet[2] = 0;
    setup_packet[3] = 1;    // DEVICE descriptor
    setup_packet[4] = 0;
    setup_packet[5] = 0;
    setup_packet[6] = 8;    // length
    setup_packet[7] = 0;

    // --- Channel config ---
    *USB_HCCHAR0 = 0;
    delay(10000);

    unsigned int hcchar = 0;
    hcchar |= (8 << 0);   // max packet size
    hcchar |= (0 << 11);  // EP0
    hcchar |= (0 << 15);  // OUT (setup stage)
    hcchar |= (0 << 18);  // full speed
    hcchar |= (0 << 22);  // addr 0

    *USB_HCCHAR0 = hcchar;

    // --- Transfer size ---
    *USB_HCTSIZ0 = (8 | (1 << 19)); // 1 packet

    // --- DMA pointer ---
    *USB_HCDMA0 = (unsigned int)setup_packet;

    terminal_write("Starting transfer...\n");

    *USB_HCCHAR0 |= (1 << 31); // enable

    delay(300000);

    terminal_write("Transfer attempted.\n");
}