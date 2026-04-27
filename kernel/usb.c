#include "usb.h"
#include "terminal.h"

#define USB_BASE 0x20980000

#define USB_GAHBCFG    ((volatile unsigned int*)(USB_BASE + 0x008))
#define USB_GUSBCFG    ((volatile unsigned int*)(USB_BASE + 0x00C))
#define USB_GRSTCTL    ((volatile unsigned int*)(USB_BASE + 0x010))
#define USB_GINTSTS    ((volatile unsigned int*)(USB_BASE + 0x014))
#define USB_GINTMSK    ((volatile unsigned int*)(USB_BASE + 0x018))
#define USB_GRXSTSP    ((volatile unsigned int*)(USB_BASE + 0x020))
#define USB_GRXFSIZ    ((volatile unsigned int*)(USB_BASE + 0x024))
#define USB_GNPTXFSIZ  ((volatile unsigned int*)(USB_BASE + 0x028))
#define USB_GNPTXSTS   ((volatile unsigned int*)(USB_BASE + 0x02C))

#define USB_HCFG       ((volatile unsigned int*)(USB_BASE + 0x400))
#define USB_HFIR       ((volatile unsigned int*)(USB_BASE + 0x404))
#define USB_HAINT      ((volatile unsigned int*)(USB_BASE + 0x414))
#define USB_HAINTMSK   ((volatile unsigned int*)(USB_BASE + 0x418))
#define USB_HPRT       ((volatile unsigned int*)(USB_BASE + 0x440))

#define USB_HCCHAR0    ((volatile unsigned int*)(USB_BASE + 0x500))
#define USB_HCSPLT0    ((volatile unsigned int*)(USB_BASE + 0x504))
#define USB_HCINT0     ((volatile unsigned int*)(USB_BASE + 0x508))
#define USB_HCINTMSK0  ((volatile unsigned int*)(USB_BASE + 0x50C))
#define USB_HCTSIZ0    ((volatile unsigned int*)(USB_BASE + 0x510))
#define USB_HCDMA0     ((volatile unsigned int*)(USB_BASE + 0x514))

#define USB_FIFO0      ((volatile unsigned int*)(USB_BASE + 0x1000))

#define GINTSTS_RXFLVL (1 << 4)

#define HCINT_XFERCOMP (1 << 0)
#define HCINT_CHHLTD   (1 << 1)
#define HCINT_AHBERR   (1 << 2)
#define HCINT_STALL    (1 << 3)
#define HCINT_NAK      (1 << 4)
#define HCINT_ACK      (1 << 5)
#define HCINT_TXERR    (1 << 7)
#define HCINT_BBLERR   (1 << 8)
#define HCINT_FRMOVRUN (1 << 9)
#define HCINT_DATATGL  (1 << 10)

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

static void flush_tx_fifo(void) {
    *USB_GRSTCTL = (1 << 5) | (0x10 << 6);
    while (*USB_GRSTCTL & (1 << 5));
}

static void flush_rx_fifo(void) {
    *USB_GRSTCTL = (1 << 4);
    while (*USB_GRSTCTL & (1 << 4));
}

static void clear_channel0(void) {
    *USB_HCCHAR0 = 0;
    *USB_HCSPLT0 = 0;
    *USB_HCINT0 = 0xFFFFFFFF;
    *USB_HCINTMSK0 = 0;
    *USB_HCTSIZ0 = 0;
    *USB_HCDMA0 = 0;
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

static void print_hcint_flags(unsigned int hcint) {
    terminal_write("Flags: ");
    if (hcint & HCINT_XFERCOMP) terminal_write("XFERCOMP ");
    if (hcint & HCINT_CHHLTD) terminal_write("CHHLTD ");
    if (hcint & HCINT_AHBERR) terminal_write("AHBERR ");
    if (hcint & HCINT_STALL) terminal_write("STALL ");
    if (hcint & HCINT_NAK) terminal_write("NAK ");
    if (hcint & HCINT_ACK) terminal_write("ACK ");
    if (hcint & HCINT_TXERR) terminal_write("TXERR ");
    if (hcint & HCINT_BBLERR) terminal_write("BBLERR ");
    if (hcint & HCINT_FRMOVRUN) terminal_write("FRMOVRUN ");
    if (hcint & HCINT_DATATGL) terminal_write("DATATGL ");
    terminal_write("\n");
}

static void start_channel(void) {
    unsigned int hcchar = *USB_HCCHAR0;

    hcchar &= ~(1u << 30);
    hcchar |= (1u << 31);

    *USB_HCCHAR0 = hcchar;
}

static unsigned int make_hcchar(int is_in) {
    unsigned int hcchar = 0;

    hcchar |= (8 << 0);       // MPS = 8
    hcchar |= (0 << 11);      // EP num = 0

    if (is_in) {
        hcchar |= (1 << 15);  // IN
    }

    // EP type CONTROL = 0 at bits 18:19
    hcchar |= (0 << 18);

    // Device address = 0 before SET_ADDRESS
    hcchar |= (0 << 22);

    // DWC2 low-speed device flag for HCCHAR is bit 17 in the Pi DWC2 layout
    if (usb_low_speed) {
        hcchar |= (1 << 17);
    }

    return hcchar;
}

static unsigned int wait_channel_event(void) {
    unsigned int hcint = 0;

    for (int i = 0; i < 2000000; i++) {
        hcint = *USB_HCINT0;

        if (hcint != 0) {
            return hcint;
        }

        if (*USB_GINTSTS & GINTSTS_RXFLVL) {
            return hcint;
        }
    }

    return hcint;
}

void usb_init(void) {
    terminal_write("USB init...\n");

    while (((*USB_GRSTCTL) & (1 << 31)) == 0);

    *USB_GRSTCTL |= 1;
    while (*USB_GRSTCTL & 1);

    delay(200000);

    // Force host mode
    *USB_GUSBCFG |= (1 << 29);
    delay(200000);

    // PIO mode, global interrupt enable
    *USB_GAHBCFG = (1 << 0);

    // Unmask lots of core interrupts for debugging
    *USB_GINTMSK = 0xFFFFFFFF;
    *USB_HAINTMSK = 0xFFFFFFFF;

    // Host frame interval
    *USB_HFIR = 48000;

    // Default host clock, will adjust after speed detect
    *USB_HCFG = 3;

    *USB_GRXFSIZ = 512;
    *USB_GNPTXFSIZ = (256 << 16) | 512;

    flush_tx_fifo();
    flush_rx_fifo();
    clear_channel0();

    // Power port
    *USB_HPRT |= (1 << 12);
    delay(500000);

    terminal_write("Resetting port...\n");

    *USB_HPRT |= (1 << 8);
    delay(800000);
    *USB_HPRT &= ~(1 << 8);
    delay(1500000);

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

    terminal_write("HCFG ");
    print_hex32(*USB_HCFG);
    terminal_write("\n");

    delay(500000);
}

void usb_test_descriptor(void) {
    terminal_write("Reading descriptor UBoot-ish...\n");

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

    clear_channel0();
    flush_tx_fifo();
    flush_rx_fifo();

    *USB_HCINT0 = 0xFFFFFFFF;
    *USB_HCINTMSK0 =
        HCINT_XFERCOMP |
        HCINT_CHHLTD |
        HCINT_AHBERR |
        HCINT_STALL |
        HCINT_NAK |
        HCINT_ACK |
        HCINT_TXERR |
        HCINT_BBLERR |
        HCINT_FRMOVRUN |
        HCINT_DATATGL;

    *USB_HCCHAR0 = make_hcchar(0);

    *USB_HCTSIZ0 =
        (8) |
        (1 << 19) |
        (3 << 29);

    terminal_write("SETUP HCCHAR ");
    print_hex32(*USB_HCCHAR0);
    terminal_write("\n");

    terminal_write("SETUP HCTSIZ ");
    print_hex32(*USB_HCTSIZ0);
    terminal_write("\n");

    terminal_write("GNPTXSTS ");
    print_hex32(*USB_GNPTXSTS);
    terminal_write("\n");

    write_fifo(setup_packet, 8);

    terminal_write("SETUP start\n");
    start_channel();

    unsigned int setup_int = wait_channel_event();

    terminal_write("SETUP HCINT ");
    print_hex32(setup_int);
    terminal_write("\n");
    print_hcint_flags(setup_int);

    terminal_write("SETUP HCCHAR after ");
    print_hex32(*USB_HCCHAR0);
    terminal_write("\n");

    terminal_write("SETUP HCTSIZ after ");
    print_hex32(*USB_HCTSIZ0);
    terminal_write("\n");

    *USB_HCINT0 = 0xFFFFFFFF;

    *USB_HCCHAR0 = make_hcchar(1);

    *USB_HCTSIZ0 =
        (8) |
        (1 << 19) |
        (2 << 29);

    terminal_write("DATA HCCHAR ");
    print_hex32(*USB_HCCHAR0);
    terminal_write("\n");

    terminal_write("DATA HCTSIZ ");
    print_hex32(*USB_HCTSIZ0);
    terminal_write("\n");

    terminal_write("DATA start\n");
    start_channel();

    unsigned int data_int = wait_channel_event();

    terminal_write("DATA HCINT ");
    print_hex32(data_int);
    terminal_write("\n");
    print_hcint_flags(data_int);

    int got_rx = 0;
    unsigned int rx_status = 0;

    for (int wait = 0; wait < 2000000; wait++) {
        if (*USB_GINTSTS & GINTSTS_RXFLVL) {
            got_rx = 1;
            rx_status = *USB_GRXSTSP;
            break;
        }
    }

    terminal_write("GINTSTS ");
    print_hex32(*USB_GINTSTS);
    terminal_write("\n");

    terminal_write("HAINT ");
    print_hex32(*USB_HAINT);
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