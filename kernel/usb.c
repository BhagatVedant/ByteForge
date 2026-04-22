#include "usb.h"
#include "terminal.h"

#define USB_BASE 0x20980000  // Pi Zero USB controller base

#define USB_GAHBCFG   ((volatile unsigned int*)(USB_BASE + 0x008))
#define USB_GUSBCFG   ((volatile unsigned int*)(USB_BASE + 0x00C))
#define USB_GRSTCTL   ((volatile unsigned int*)(USB_BASE + 0x010))
#define USB_GINTSTS   ((volatile unsigned int*)(USB_BASE + 0x014))
#define USB_GINTMSK   ((volatile unsigned int*)(USB_BASE + 0x018))
#define USB_HCFG      ((volatile unsigned int*)(USB_BASE + 0x400))
#define USB_HPRT      ((volatile unsigned int*)(USB_BASE + 0x440))

static void delay(int count) {
    while (count--) {
        asm volatile("nop");
    }
}

void usb_init(void) {
    terminal_write("Initializing USB...\n");

    // Core soft reset
    *USB_GRSTCTL |= (1 << 0);
    while (*USB_GRSTCTL & (1 << 0));

    delay(100000);

    // Force host mode
    *USB_GUSBCFG |= (1 << 29);

    delay(100000);

    // Enable global interrupts
    *USB_GAHBCFG |= 1;

    // Host config
    *USB_HCFG |= 0x3;

    delay(100000);

    // --- NEW PART STARTS HERE ---

    // Power the port
    *USB_HPRT |= (1 << 12);  // Port Power

    delay(200000);

    // Reset the port
    *USB_HPRT |= (1 << 8);   // Port Reset

    delay(200000);

    *USB_HPRT &= ~(1 << 8);  // Clear reset

    delay(200000);

    terminal_write("USB port powered and reset.\n");

    // --- CHECK AGAIN ---
    unsigned int port = *USB_HPRT;

    if (port & (1 << 0)) {
        terminal_write("USB device detected.\n");
    } else {
        terminal_write("No USB device.\n");
    }
}