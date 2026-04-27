#include "usb.h"
#include "terminal.h"

#define USB_BASE 0x20980000

#define USB_GAHBCFG    ((volatile unsigned int*)(USB_BASE + 0x008))
#define USB_GUSBCFG    ((volatile unsigned int*)(USB_BASE + 0x00C))
#define USB_GRSTCTL    ((volatile unsigned int*)(USB_BASE + 0x010))
#define USB_GRXFSIZ    ((volatile unsigned int*)(USB_BASE + 0x024))
#define USB_GNPTXFSIZ  ((volatile unsigned int*)(USB_BASE + 0x028))
#define USB_GINTSTS    ((volatile unsigned int*)(USB_BASE + 0x014))

#define USB_HCFG       ((volatile unsigned int*)(USB_BASE + 0x400))
#define USB_HPRT       ((volatile unsigned int*)(USB_BASE + 0x440))

#define USB_HCCHAR0    ((volatile unsigned int*)(USB_BASE + 0x500))
#define USB_HCINT0     ((volatile unsigned int*)(USB_BASE + 0x508))
#define USB_HCTSIZ0    ((volatile unsigned int*)(USB_BASE + 0x510))

#define USB_FIFO0      ((volatile unsigned int*)(USB_BASE + 0x1000))

static unsigned char setup_packet[8];
static unsigned char buffer[64];

static void delay(int c) {
    while (c--) asm volatile("nop");
}

static void print_byte(unsigned char b) {
    char hex[] = "0123456789ABCDEF";
    char out[3];
    out[0] = hex[(b >> 4) & 0xF];
    out[1] = hex[b & 0xF];
    out[2] = 0;
    terminal_write(out);
}

void usb_init(void) {
    terminal_write("USB init...\n");

    while(((*USB_GRSTCTL)&(1<<31)) == 0);

    *USB_GRSTCTL |= 1;
    while(*USB_GRSTCTL & 1);

    delay(100000);

    *USB_GUSBCFG |= (1<<29);

    *USB_GAHBCFG = (1<<0);  // NO DMA

    *USB_HCFG = 3;

    *USB_GRXFSIZ = 512;
    *USB_GNPTXFSIZ = (256<<16) | 512;

    // power
    *USB_HPRT |= (1<<12);
    delay(200000);

    // reset
    *USB_HPRT |= (1<<8);
    delay(200000);
    *USB_HPRT &= ~(1<<8);
    delay(200000);

    if (*USB_HPRT & 1)
        terminal_write("Device connected\n");
    else
        terminal_write("No device\n");
}

static void write_fifo(unsigned char* data, int len) {
    volatile unsigned int* fifo = USB_FIFO0;

    for (int i = 0; i < len; i += 4) {
        unsigned int val =
            data[i] |
            (data[i+1] << 8) |
            (data[i+2] << 16) |
            (data[i+3] << 24);

        *fifo = val;
    }
}

static void read_fifo(unsigned char* out, int len) {
    volatile unsigned int* fifo = USB_FIFO0;

    for (int i = 0; i < len; i += 4) {
        unsigned int val = *fifo;

        out[i]   = val & 0xFF;
        out[i+1] = (val >> 8) & 0xFF;
        out[i+2] = (val >> 16) & 0xFF;
        out[i+3] = (val >> 24) & 0xFF;
    }
}

void usb_test_descriptor(void) {
    terminal_write("Reading descriptor...\n");

    for (int i=0;i<64;i++) buffer[i]=0;

    // setup packet
    setup_packet[0] = 0x80;
    setup_packet[1] = 0x06;
    setup_packet[2] = 0x00;
    setup_packet[3] = 0x01;
    setup_packet[4] = 0;
    setup_packet[5] = 0;
    setup_packet[6] = 18;
    setup_packet[7] = 0;

    *USB_HCINT0 = 0xFFFFFFFF;

    // SETUP stage
    *USB_HCCHAR0 = (8 << 0);
    *USB_HCTSIZ0 = (8) | (1 << 19) | (3 << 29);

    write_fifo(setup_packet, 8);

    *USB_HCCHAR0 |= (1<<31);
    delay(200000);

    terminal_write("SETUP done\n");

    // DATA stage (IN)
    *USB_HCINT0 = 0xFFFFFFFF;

    *USB_HCCHAR0 = (8 << 0) | (1 << 15);
    *USB_HCTSIZ0 = (18) | (1 << 19) | (2 << 29);

    *USB_HCCHAR0 |= (1<<31);
    delay(200000);

    read_fifo(buffer, 18);

    terminal_write("Descriptor:\n");

    for (int i=0;i<18;i++) {
        print_byte(buffer[i]);
        terminal_write(" ");
    }

    terminal_write("\n");
}