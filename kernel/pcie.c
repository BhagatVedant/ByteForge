#include "pcie.h"
#include "terminal.h"
#include <stdint.h>

#define TEXT_COLOR       0x00E6E6E6
#define PCIE_COLOR       0x0000CCFF
#define OK_COLOR         0x0000FF88
#define BAD_COLOR        0x00FF6666

#define PCIE_RC_BASE             0x1000120000UL

#define PCIE_EXT_CFG_DATA        0x8000
#define PCIE_EXT_CFG_INDEX       0x9000
#define PCIE_MISC_PCIE_STATUS    0x4068

#define PCIE_STATUS_DL_ACTIVE    0x20
#define PCIE_STATUS_PHYLINKUP    0x10

#define PCI_COMMAND_OFFSET       0x04
#define PCI_BAR0_OFFSET          0x10
#define PCI_BAR1_OFFSET          0x14

#define PCI_COMMAND_MEMORY       0x0002
#define PCI_COMMAND_MASTER       0x0004

#define RP1_VENDOR_ID            0x1DE4
#define RP1_DEVICE_ID            0x0001

#define RP1_SYSINFO_OFFSET       0x000000
#define RP1_PCIE_APBS_OFFSET     0x108000
#define RP1_PCIE_INTS_OFFSET     0x1B4

#define RP1_USB0_OFFSET          0x200000
#define RP1_USB1_OFFSET          0x300000

#define DWC3_GCTL                0xC110
#define DWC3_GSTS                0xC118
#define DWC3_GSNPSID             0xC120

typedef struct {
    uint8_t bus;
    uint8_t dev;
    uint8_t fn;
} rp1_bdf_t;

static uint32_t mmio_read32(uint64_t address) {
    return *(volatile uint32_t *)(uintptr_t)address;
}

static void mmio_write32(uint64_t address, uint32_t value) {
    *(volatile uint32_t *)(uintptr_t)address = value;
}

static void memory_barrier(void) {
    __asm__ volatile ("dsb sy" ::: "memory");
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

static uint32_t make_ecam_index(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t offset) {
    uint8_t devfn = (dev << 3) | fn;

    return ((uint32_t)bus << 20) |
           ((uint32_t)devfn << 12) |
           (offset & 0x0FFF);
}

static uint32_t pcie_cfg_read32(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t offset) {
    uint32_t aligned_offset = offset & ~3U;
    uint32_t index = make_ecam_index(bus, dev, fn, aligned_offset);

    mmio_write32(PCIE_RC_BASE + PCIE_EXT_CFG_INDEX, index);
    memory_barrier();

    return mmio_read32(PCIE_RC_BASE + PCIE_EXT_CFG_DATA + aligned_offset);
}

static void pcie_cfg_write32(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t offset, uint32_t value) {
    uint32_t aligned_offset = offset & ~3U;
    uint32_t index = make_ecam_index(bus, dev, fn, aligned_offset);

    mmio_write32(PCIE_RC_BASE + PCIE_EXT_CFG_INDEX, index);
    memory_barrier();

    mmio_write32(PCIE_RC_BASE + PCIE_EXT_CFG_DATA + aligned_offset, value);
    memory_barrier();
}

static uint16_t pcie_cfg_read16(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t offset) {
    uint32_t value = pcie_cfg_read32(bus, dev, fn, offset & ~3U);
    uint32_t shift = (offset & 2U) ? 16 : 0;

    return (uint16_t)((value >> shift) & 0xFFFF);
}

static void pcie_cfg_write16(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t offset, uint16_t value) {
    uint32_t old_value = pcie_cfg_read32(bus, dev, fn, offset & ~3U);
    uint32_t shift = (offset & 2U) ? 16 : 0;
    uint32_t mask = 0xFFFFU << shift;
    uint32_t new_value = (old_value & ~mask) | ((uint32_t)value << shift);

    pcie_cfg_write32(bus, dev, fn, offset & ~3U, new_value);
}

static int pcie_link_up(void) {
    uint32_t status = mmio_read32(PCIE_RC_BASE + PCIE_MISC_PCIE_STATUS);

    terminal_set_color(PCIE_COLOR);
    terminal_write("PCIe: RC status ");
    terminal_set_color(TEXT_COLOR);
    print_hex32(status);

    terminal_write(" dl=");
    terminal_write((status & PCIE_STATUS_DL_ACTIVE) ? "1" : "0");

    terminal_write(" phy=");
    terminal_write((status & PCIE_STATUS_PHYLINKUP) ? "1" : "0");

    terminal_write("\n");

    return (status & (PCIE_STATUS_DL_ACTIVE | PCIE_STATUS_PHYLINKUP)) ==
           (PCIE_STATUS_DL_ACTIVE | PCIE_STATUS_PHYLINKUP);
}

static int find_rp1(rp1_bdf_t *out) {
    for (uint8_t bus = 1; bus <= 3; bus++) {
        for (uint8_t dev = 0; dev < 32; dev++) {
            for (uint8_t fn = 0; fn < 8; fn++) {
                uint32_t vendor_device = pcie_cfg_read32(bus, dev, fn, 0x00);

                uint16_t vendor = vendor_device & 0xFFFF;
                uint16_t device = (vendor_device >> 16) & 0xFFFF;

                if (vendor == 0xFFFF || vendor == 0x0000) {
                    continue;
                }

                terminal_set_color(TEXT_COLOR);
                terminal_write("PCIe cfg ");
                print_hex32(bus);
                terminal_write(":");
                print_hex32(dev);
                terminal_write(".");
                print_hex32(fn);
                terminal_write(" VID:DID=");
                print_hex32(vendor_device);
                terminal_write("\n");

                if (vendor == RP1_VENDOR_ID && device == RP1_DEVICE_ID) {
                    out->bus = bus;
                    out->dev = dev;
                    out->fn = fn;
                    return 1;
                }
            }
        }
    }

    return 0;
}

static void enable_rp1_pci(rp1_bdf_t bdf) {
    uint16_t command = pcie_cfg_read16(bdf.bus, bdf.dev, bdf.fn, PCI_COMMAND_OFFSET);
    uint16_t new_command = command | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;

    pcie_cfg_write16(bdf.bus, bdf.dev, bdf.fn, PCI_COMMAND_OFFSET, new_command);

    terminal_set_color(PCIE_COLOR);
    terminal_write("PCIe: command ");
    terminal_set_color(TEXT_COLOR);
    print_hex32(command);
    terminal_write(" -> ");
    print_hex32(new_command);
    terminal_write("\n");
}

static uint64_t read_bar(rp1_bdf_t bdf, uint16_t offset) {
    uint32_t low = pcie_cfg_read32(bdf.bus, bdf.dev, bdf.fn, offset);

    if (low & 0x1) {
        return low & ~0x3ULL;
    }

    uint32_t type = (low >> 1) & 0x3;

    if (type == 0x2) {
        uint32_t high = pcie_cfg_read32(bdf.bus, bdf.dev, bdf.fn, offset + 4);
        return ((uint64_t)high << 32) | (uint64_t)(low & ~0xFULL);
    }

    return low & ~0xFULL;
}

static void probe_dwc3(const char *name, uint64_t base) {
    uint32_t gctl = mmio_read32(base + DWC3_GCTL);
    uint32_t gsts = mmio_read32(base + DWC3_GSTS);
    uint32_t gsnpsid = mmio_read32(base + DWC3_GSNPSID);

    terminal_set_color(PCIE_COLOR);
    terminal_write(name);
    terminal_write(" at ");
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

    if ((gsnpsid & 0xFFFF0000) == 0x55330000) {
        terminal_set_color(OK_COLOR);
        terminal_write("  Result: DWC3 found.\n");
    } else {
        terminal_set_color(BAD_COLOR);
        terminal_write("  Result: DWC3 not confirmed.\n");
    }

    terminal_set_color(TEXT_COLOR);
}

void rp1_probe_minimal(void) {
    rp1_bdf_t rp1;

    terminal_set_color(PCIE_COLOR);
    terminal_write("PCIe: RP1 BAR probe start.\n");

    if (!pcie_link_up()) {
        terminal_set_color(BAD_COLOR);
        terminal_write("PCIe: link not up. Stop.\n");
        terminal_set_color(TEXT_COLOR);
        return;
    }

    if (!find_rp1(&rp1)) {
        terminal_set_color(BAD_COLOR);
        terminal_write("PCIe: RP1 1de4:0001 not found.\n");
        terminal_set_color(TEXT_COLOR);
        return;
    }

    terminal_set_color(OK_COLOR);
    terminal_write("PCIe: RP1 found.\n");
    terminal_set_color(TEXT_COLOR);

    enable_rp1_pci(rp1);

    uint64_t bar0 = read_bar(rp1, PCI_BAR0_OFFSET);
    uint64_t bar1 = read_bar(rp1, PCI_BAR1_OFFSET);

    terminal_set_color(PCIE_COLOR);
    terminal_write("RP1 BAR0: ");
    terminal_set_color(TEXT_COLOR);
    print_hex64(bar0);
    terminal_write("\n");

    terminal_set_color(PCIE_COLOR);
    terminal_write("RP1 BAR1: ");
    terminal_set_color(TEXT_COLOR);
    print_hex64(bar1);
    terminal_write("\n");

    if (bar1 == 0 || bar1 == 0xFFFFFFFFFFFFFFF0UL) {
        terminal_set_color(BAD_COLOR);
        terminal_write("RP1: BAR1 invalid. Firmware/enumeration incomplete.\n");
        terminal_set_color(TEXT_COLOR);
        return;
    }

    uint64_t sysinfo = bar1 + RP1_SYSINFO_OFFSET;
    uint64_t pcie_apbs = bar1 + RP1_PCIE_APBS_OFFSET;
    uint64_t usb0 = bar1 + RP1_USB0_OFFSET;
    uint64_t usb1 = bar1 + RP1_USB1_OFFSET;

    uint32_t chip_id = mmio_read32(sysinfo + 0x00);
    uint32_t platform = mmio_read32(sysinfo + 0x04);
    uint32_t ints = mmio_read32(pcie_apbs + RP1_PCIE_INTS_OFFSET);

    terminal_set_color(PCIE_COLOR);
    terminal_write("RP1 sysinfo ");
    print_hex64(sysinfo);
    terminal_write("\n");

    terminal_set_color(TEXT_COLOR);
    terminal_write("  CHIP_ID:  ");
    print_hex32(chip_id);
    terminal_write("\n");

    terminal_write("  PLATFORM: ");
    print_hex32(platform);
    terminal_write("\n");

    terminal_write("  INTS:     ");
    print_hex32(ints);
    terminal_write("\n");

    probe_dwc3("USB0 DWC3", usb0);
    probe_dwc3("USB1 DWC3", usb1);

    terminal_set_color(PCIE_COLOR);
    terminal_write("PCIe: RP1 BAR probe done.\n");
    terminal_set_color(TEXT_COLOR);
}