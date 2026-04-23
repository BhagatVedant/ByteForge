#include "usb.h"
#include "terminal.h"

#define USB_BASE 0x20980000

#define USB_GAHBCFG    ((volatile unsigned int*)(USB_BASE + 0x008))
#define USB_GUSBCFG    ((volatile unsigned int*)(USB_BASE + 0x00C))
#define USB_GRSTCTL    ((volatile unsigned int*)(USB_BASE + 0x010))
#define USB_GINTSTS    ((volatile unsigned int*)(USB_BASE + 0x014))
#define USB_GINTMSK    ((volatile unsigned int*)(USB_BASE + 0x018))
#define USB_GRXFSIZ    ((volatile unsigned int*)(USB_BASE + 0x024))
#define USB_GNPTXFSIZ  ((volatile unsigned int*)(USB_BASE + 0x028))
#define USB_GNPTXSTS   ((volatile unsigned int*)(USB_BASE + 0x02C))
#define USB_HPTXFSIZ   ((volatile unsigned int*)(USB_BASE + 0x100))
#define USB_GNPTXFSIZ  ((volatile unsigned int*)(USB_BASE + 0x028))

#define USB_HCFG       ((volatile unsigned int*)(USB_BASE + 0x400))
#define USB_HPRT       ((volatile unsigned int*)(USB_BASE + 0x440))

#define USB_HAINT      ((volatile unsigned int*)(USB_BASE + 0x414))
#define USB_HAINTMSK   ((volatile unsigned int*)(USB_BASE + 0x418))

#define USB_HCCHAR0    ((volatile unsigned int*)(USB_BASE + 0x500))
#define USB_HCSPLT0    ((volatile unsigned int*)(USB_BASE + 0x504))
#define USB_HCINT0     ((volatile unsigned int*)(USB_BASE + 0x508))
#define USB_HCINTMSK0  ((volatile unsigned int*)(USB_BASE + 0x50C))
#define USB_HCTSIZ0    ((volatile unsigned int*)(USB_BASE + 0x510))
#define USB_HCDMA0     ((volatile unsigned int*)(USB_BASE + 0x514))

#define USB_DATA_FIFO0 ((volatile unsigned int*)(USB_BASE + 0x1000))

static unsigned char setup_packet[8];

static void delay(int count) {
    while (count--) {
        asm volatile("nop");
    }
}

static void print_hex(unsigned int val) {
    char buf[11];
    const char *hex = "0123456789ABCDEF";

    buf[0] = '0';
    buf[1] = 'x';

    for (int i = 0; i < 8; i++) {
        buf[2 + i] = hex[(val >> (28 - i * 4)) & 0xF];
    }

    buf[10] = '\0';
    terminal_write(buf);
}

static void usb_flush_tx_fifo(void) {
    *USB_GRSTCTL = (1 << 5) | (0x10 << 6);
    while (*USB_GRSTCTL & (1 << 5)) {
    }
}

static void usb_flush_rx_fifo(void) {
    *USB_GRSTCTL = (1 << 4);
    while (*USB_GRSTCTL & (1 << 4)) {
    }
}

static void usb_channel0_clear(void) {
    *USB_HCCHAR0 = 0;
    *USB_HCSPLT0 = 0;
    *USB_HCINT0 = 0xFFFFFFFF;
    *USB_HCINTMSK0 = 0;
    *USB_HCTSIZ0 = 0;
    *USB_HCDMA0 = 0;
}

static void usb_channel0_start(void) {
    unsigned int hcchar = *USB_HCCHAR0;

    hcchar &= ~(1u << 30);
    hcchar |=  (1u << 31);

    *USB_HCCHAR0 = hcchar;
}

static void usb_write_setup_fifo(void) {
    unsigned int word0 =
        ((unsigned int)setup_packet[0]) |
        ((unsigned int)setup_packet[1] << 8) |
        ((unsigned int)setup_packet[2] << 16) |
        ((unsigned int)setup_packet[3] << 24);

    unsigned int word1 =
        ((unsigned int)setup_packet[4]) |
        ((unsigned int)setup_packet[5] << 8) |
        ((unsigned int)setup_packet[6] << 16) |
        ((unsigned int)setup_packet[7] << 24);

    *USB_DATA_FIFO0 = word0;
    *USB_DATA_FIFO0 = word1;
}

void usb_init(void) {
    terminal_write("Initializing USB...\n");

    while (((*USB_GRSTCTL) & (1 << 31)) == 0) {
    }

    *USB_GRSTCTL |= 1;
    while (*USB_GRSTCTL & 1) {
    }

    delay(100000);

    *USB_GUSBCFG |= (1 << 29);
    delay(100000);

    // global interrupt on, DMA off for this experiment
    *USB_GAHBCFG = (1 << 0);

    *USB_GINTMSK = 0xFFFFFFFF;
    *USB_HAINTMSK = 0xFFFFFFFF;

    *USB_HCFG = 3;

    *USB_GRXFSIZ = 512;
    *USB_GNPTXFSIZ = (256 << 16) | 512;
    *USB_HPTXFSIZ = (256 << 16) | 768;

    usb_flush_tx_fifo();
    usb_flush_rx_fifo();
    usb_channel0_clear();

    *USB_HPRT |= (1 << 12);
    delay(200000);

    *USB_HPRT |= (1 << 8);
    delay(300000);
    *USB_HPRT &= ~(1 << 8);
    delay(300000);

    terminal_write("USB_HPRT ");
    print_hex(*USB_HPRT);
    terminal_write("\n");

    if (*USB_HPRT & 1) {
        terminal_write("USB device detected.\n");
    } else {
        terminal_write("No USB device.\n");
    }
}

void usb_enumerate(void) {
    terminal_write("PIO control transfer experiment\n");

    setup_packet[0] = 0x80;
    setup_packet[1] = 0x06;
    setup_packet[2] = 0x00;
    setup_packet[3] = 0x01;
    setup_packet[4] = 0x00;
    setup_packet[5] = 0x00;
    setup_packet[6] = 0x08;
    setup_packet[7] = 0x00;

    usb_channel0_clear();

    // unmask common channel interrupts so we can at least see movement
    *USB_HCINTMSK0 = 0xFFFFFFFF;

    // EP0, addr 0, max packet 8, OUT
    *USB_HCCHAR0 =
        (8 << 0)  |
        (0 << 11) |
        (0 << 15) |
        (0 << 18) |
        (0 << 22);

    // setup packet: 8 bytes, 1 packet, SETUP PID
    *USB_HCTSIZ0 =
        (8)         |
        (1 << 19)   |
        (3 << 29);

    terminal_write("Before FIFO write\n");
    terminal_write("GNPTXSTS ");
    print_hex(*USB_GNPTXSTS);
    terminal_write("\n");

    usb_write_setup_fifo();

    terminal_write("Starting SETUP stage\n");
    usb_channel0_start();

    delay(500000);

    terminal_write("After SETUP\n");
    terminal_write("HCINT0   ");
    print_hex(*USB_HCINT0);
    terminal_write("\n");
    terminal_write("HCTSIZ0  ");
    print_hex(*USB_HCTSIZ0);
    terminal_write("\n");
    terminal_write("GINTSTS  ");
    print_hex(*USB_GINTSTS);
    terminal_write("\n");
    terminal_write("HAINT    ");
    print_hex(*USB_HAINT);
    terminal_write("\n");
}