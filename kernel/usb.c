#include "usb.h"
#include "terminal.h"

#define USB_BASE 0x20980000

#define USB_GAHBCFG    ((volatile unsigned int*)(USB_BASE + 0x008))
#define USB_GUSBCFG    ((volatile unsigned int*)(USB_BASE + 0x00C))
#define USB_GRSTCTL    ((volatile unsigned int*)(USB_BASE + 0x010))
#define USB_GINTSTS    ((volatile unsigned int*)(USB_BASE + 0x014))
#define USB_GRXSTSP    ((volatile unsigned int*)(USB_BASE + 0x020))
#define USB_GRXFSIZ    ((volatile unsigned int*)(USB_BASE + 0x024))
#define USB_GNPTXFSIZ  ((volatile unsigned int*)(USB_BASE + 0x028))

#define USB_HCFG       ((volatile unsigned int*)(USB_BASE + 0x400))
#define USB_HPRT       ((volatile unsigned int*)(USB_BASE + 0x440))

#define USB_HCCHAR0    ((volatile unsigned int*)(USB_BASE + 0x500))
#define USB_HCINT0     ((volatile unsigned int*)(USB_BASE + 0x508))
#define USB_HCINTMSK0  ((volatile unsigned int*)(USB_BASE + 0x50C))
#define USB_HCTSIZ0    ((volatile unsigned int*)(USB_BASE + 0x510))

#define USB_FIFO0      ((volatile unsigned int*)(USB_BASE + 0x1000))

#define GINTSTS_RXFLVL (1 << 4)

static unsigned char setup_packet[8];
static unsigned char buffer[64];
static int usb_low_speed = 0;

static void delay(int c) {
    while (c--) {
        asm volatile("nop");
    }
}

static void print_hex32(unsigned int v) {
    char hex[] = "0123456789ABCDEF";
    char out[11];

    out[0] = '0';
    out[1] = 'x';

    for (int i = 0; i < 8; i++) {
        out[2 + i] = hex[(v >> (28 - i * 4)) & 0xF];
    }

    out[10] = 0;
    terminal_write(out);
}

static void print_byte(unsigned char b) {
    char hex[] = "0123456789ABCDEF";
    char out[3];

    out[0] = hex[(b >> 4) & 0xF];
    out[1] = hex[b & 0xF];
    out[2] = 0;

    terminal_write(out);
}

static void print_num(unsigned int n) {
    char buf[12];
    int i = 0;

    if (n == 0) {
        terminal_write("0");
        return;
    }

    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }

    while (i > 0) {
        char c[2];
        c[0] = buf[--i];
        c[1] = 0;
        terminal_write(c);
    }
}

static void write_fifo(unsigned char *data, int len) {
    for (int i = 0; i < len; i += 4) {
        unsigned int val =
            ((unsigned int)data[i]) |
            ((unsigned int)data[i + 1] << 8) |
            ((unsigned int)data[i + 2] << 16) |
            ((unsigned int)data[i + 3] << 24);

        *USB_FIFO0 = val;
    }
}

static void start_channel(void) {
    unsigned int hcchar = *USB_HCCHAR0;

    hcchar &= ~(1u << 30);
    hcchar |=  (1u << 31);

    *USB_HCCHAR0 = hcchar;
}

static void configure_ep0_out(void) {
    unsigned int hcchar = 0;

    hcchar |= (8 << 0);      // max packet size = 8
    hcchar |= (0 << 11);     // endpoint number = 0
    hcchar |= (0 << 15);     // OUT
    hcchar |= (0 << 18);     // endpoint type = control
    hcchar |= (0 << 22);     // device address = 0

    if (usb_low_speed) {
        hcchar |= (1 << 17); // low speed device
    }

    *USB_HCCHAR0 = hcchar;
}

static void configure_ep0_in(void) {
    unsigned int hcchar = 0;

    hcchar |= (8 << 0);      // max packet size = 8
    hcchar |= (0 << 11);     // endpoint number = 0
    hcchar |= (1 << 15);     // IN
    hcchar |= (0 << 18);     // endpoint type = control
    hcchar |= (0 << 22);     // device address = 0

    if (usb_low_speed) {
        hcchar |= (1 << 17); // low speed device
    }

    *USB_HCCHAR0 = hcchar;
}

void usb_init(void) {
    terminal_write("USB init...\n");

    while (((*USB_GRSTCTL) & (1 << 31)) == 0);

    *USB_GRSTCTL |= 1;
    while (*USB_GRSTCTL & 1);

    delay(200000);

    *USB_GUSBCFG |= (1 << 29);

    // PIO mode, no DMA
    *USB_GAHBCFG = (1 << 0);

    *USB_HCFG = 3;

    *USB_GRXFSIZ = 512;
    *USB_GNPTXFSIZ = (256 << 16) | 512;

    *USB_HPRT |= (1 << 12);
    delay(300000);

    terminal_write("Resetting port...\n");

    *USB_HPRT |= (1 << 8);
    delay(500000);
    *USB_HPRT &= ~(1 << 8);

    delay(1000000);

    unsigned int hprt = *USB_HPRT;

    terminal_write("HPRT ");
    print_hex32(hprt);
    terminal_write("\n");

    if (hprt & 1) {
        terminal_write("Device connected\n");
    } else {
        terminal_write("No device\n");
    }

    unsigned int speed = (hprt >> 17) & 3;

    terminal_write("Speed ");

    if (speed == 2) {
        terminal_write("Low\n");
        usb_low_speed = 1;
        *USB_HCFG = 2;
    } else if (speed == 1) {
        terminal_write("Full\n");
        usb_low_speed = 0;
        *USB_HCFG = 3;
    } else if (speed == 0) {
        terminal_write("High\n");
        usb_low_speed = 0;
        *USB_HCFG = 3;
    } else {
        terminal_write("Unknown\n");
        usb_low_speed = 0;
    }

    delay(500000);
}

void usb_test_descriptor(void) {
    terminal_write("Reading descriptor...\n");

    for (int i = 0; i < 64; i++) {
        buffer[i] = 0;
    }

    setup_packet[0] = 0x80;
    setup_packet[1] = 0x06;
    setup_packet[2] = 0x00;
    setup_packet[3] = 0x01;
    setup_packet[4] = 0x00;
    setup_packet[5] = 0x00;
    setup_packet[6] = 0x08;
    setup_packet[7] = 0x00;

    *USB_HCINT0 = 0xFFFFFFFF;
    *USB_HCINTMSK0 = 0xFFFFFFFF;

    configure_ep0_out();

    *USB_HCTSIZ0 =
        (8) |
        (1 << 19) |
        (3 << 29);      // SETUP PID

    write_fifo(setup_packet, 8);

    terminal_write("SETUP start\n");
    start_channel();

    delay(500000);

    terminal_write("SETUP HCINT ");
    print_hex32(*USB_HCINT0);
    terminal_write("\n");

    *USB_HCINT0 = 0xFFFFFFFF;

    configure_ep0_in();

    *USB_HCTSIZ0 =
        (8) |
        (1 << 19) |
        (2 << 29);      // DATA1 PID

    terminal_write("DATA start\n");
    start_channel();

    int got_rx = 0;
    unsigned int rx_status = 0;

    for (int wait = 0; wait < 1000000; wait++) {
        if (*USB_GINTSTS & GINTSTS_RXFLVL) {
            got_rx = 1;
            rx_status = *USB_GRXSTSP;
            break;
        }
    }

    terminal_write("DATA HCINT ");
    print_hex32(*USB_HCINT0);
    terminal_write("\n");

    terminal_write("GINTSTS ");
    print_hex32(*USB_GINTSTS);
    terminal_write("\n");

    if (!got_rx) {
        terminal_write("No RX packet\n");
    } else {
        unsigned int channel = rx_status & 0xF;
        unsigned int byte_count = (rx_status >> 4) & 0x7FF;
        unsigned int packet_status = (rx_status >> 17) & 0xF;

        terminal_write("RX status ");
        print_hex32(rx_status);
        terminal_write("\n");

        terminal_write("RX channel ");
        print_num(channel);
        terminal_write("\n");

        terminal_write("RX bytes ");
        print_num(byte_count);
        terminal_write("\n");

        terminal_write("RX pktsts ");
        print_num(packet_status);
        terminal_write("\n");

        int index = 0;

        while (index < (int)byte_count && index < 64) {
            unsigned int val = *USB_FIFO0;

            buffer[index++] = val & 0xFF;
            if (index < (int)byte_count) buffer[index++] = (val >> 8) & 0xFF;
            if (index < (int)byte_count) buffer[index++] = (val >> 16) & 0xFF;
            if (index < (int)byte_count) buffer[index++] = (val >> 24) & 0xFF;
        }
    }

    terminal_write("Descriptor:\n");

    for (int i = 0; i < 8; i++) {
        print_byte(buffer[i]);
        terminal_write(" ");
    }

    terminal_write("\n");
}