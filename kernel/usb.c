#include "usb.h"
#include "terminal.h"
#include <stdint.h>

#define USB_DEBUG_COLOR 0x0000CCFF
#define USB_OK_COLOR    0x0000FF88
#define USB_BAD_COLOR   0x00FF6666
#define TEXT_COLOR      0x00E6E6E6

/*
    From Pi 5 DTB:

    RP1 internal:
      usb0 = 0xC040200000
      usb1 = 0xC040300000

    RP1 ranges map internal 0xC040000000 to PCIe window.
    CPU-visible result:
      usb0 = 0x1F00200000
      usb1 = 0x1F00300000
*/

#define RP1_USB0_CPU_BASE   0x1F00200000UL
#define RP1_USB1_CPU_BASE   0x1F00300000UL

static int usb_initialized = 0;

static uint32_t mmio_read32(uint64_t address) {
    return *(volatile uint32_t *)(uintptr_t)address;
}

static void print_hex_digit(uint32_t value) {
    value &= 0xF;

    if (value < 10) {
        terminal_write_char('0' + value);
    } else {
        terminal_write_char('A' + (value - 10));
    }
}

static void print_hex32(uint32_t value) {
    terminal_write("0x");

    for (int shift = 28; shift >= 0; shift -= 4) {
        print_hex_digit(value >> shift);
    }
}

static void print_hex64(uint64_t value) {
    print_hex32((uint32_t)(value >> 32));
    terminal_write("_");
    print_hex32((uint32_t)value);
}

static int looks_like_dwc3(uint32_t gsnpsid) {
    /*
        DWC3 GSNPSID usually starts with ASCII-ish Synopsys ID 0x5533....
    */
    return (gsnpsid & 0xFFFF0000) == 0x55330000;
}

static void probe_dwc3_at(uint64_t base) {
    uint32_t gctl    = mmio_read32(base + 0xC110);
    uint32_t gsts    = mmio_read32(base + 0xC118);
    uint32_t gsnpsid = mmio_read32(base + 0xC120);

    terminal_set_color(USB_DEBUG_COLOR);
    terminal_write("USB: DWC3 probe at ");
    print_hex64(base);
    terminal_write("\n");

    terminal_set_color(TEXT_COLOR);
    terminal_write("  GCTL:    ");
    print_hex32(gctl);
    terminal_write("\n");

    terminal_write("  GSTS:    ");
    print_hex32(gsts);
    terminal_write("\n");

    terminal_write("  GSNPSID: ");
    print_hex32(gsnpsid);
    terminal_write("\n");

    if (looks_like_dwc3(gsnpsid)) {
        terminal_set_color(USB_OK_COLOR);
        terminal_write("  Result: DWC3 controller found.\n");
    } else if (gsnpsid == 0xFFFFFFFF) {
        terminal_set_color(USB_BAD_COLOR);
        terminal_write("  Result: no response at this CPU address.\n");
    } else {
        terminal_set_color(USB_BAD_COLOR);
        terminal_write("  Result: response, but not confirmed DWC3.\n");
    }

    terminal_set_color(TEXT_COLOR);
}

void usb_init(void) {
    usb_initialized = 1;

    terminal_set_color(USB_DEBUG_COLOR);
    terminal_write("USB: Pi 5 RP1 DWC3 CPU-window probe.\n");

    probe_dwc3_at(RP1_USB0_CPU_BASE);
    probe_dwc3_at(RP1_USB1_CPU_BASE);

    terminal_set_color(USB_DEBUG_COLOR);
    terminal_write("USB: probe done.\n");

    terminal_set_color(TEXT_COLOR);
}

void usb_poll(void) {
    if (!usb_initialized) {
        return;
    }
}