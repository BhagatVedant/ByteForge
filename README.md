# ByteForge

ByteForge is a bare-metal kernel project exploring low-level systems programming on Raspberry Pi hardware, written primarily in C and AArch64/ARM assembly.

## Current Target

**Raspberry Pi 5 / AArch64**

The canonical version on `main` targets Raspberry Pi 5. It is based on the known-good Pi 5 checkpoint `25e238c` (`test works`; sometimes previously referenced with the mistyped short hash `25e238e`).

## What Works on Raspberry Pi 5

The stable checkpoint provides:

- bare-metal AArch64 startup, including secondary-core parking, stack setup, and BSS initialization
- Raspberry Pi 5 / BCM2712 mailbox access
- a 1024x768, 32-bit framebuffer initialized through the firmware property interface
- bitmap text rendering, terminal output, scrolling, colors, and a blinking cursor
- a command-dispatch shell with system-information and text commands
- a queued character-input abstraction used by the built-in boot demonstration
- an in-memory storage workflow with embedded sample files and explicit demo mount/list/open/cat operations

The input and storage layers in this checkpoint are demonstrations. They do not represent a physical USB keyboard or USB mass-storage driver.

## Project Evolution

ByteForge originally began on Raspberry Pi Zero as a bare-metal OS/kernel project. That version was the original working final submission and remains a legitimate project milestone. It is preserved on `legacy/pi-zero-final` and tagged `v1.0-final-submission`.

The project was later ported to Raspberry Pi 5 and AArch64. The known-good Pi 5 port is now the canonical version on `main`. These targets reflect intentional project evolution, not conflicting hardware documentation.

## Experimental USB / xHCI Work

Later Raspberry Pi 5 development explored native USB keyboard support through RP1, PCIe, and xHCI. That separate work includes investigation and debugging around PCIe configuration, MMIO, 64-bit BAR handling, BCM2712/RP1 address translation, USB/xHCI data structures, and HID report parsing.

Full physical USB keyboard support is **not complete**. Those components are not features of stable `main`; the experimental commits are preserved on `pi5-usb-keyboard` (with the alias `experimental/pi5-usb-keyboard`).

## Raspberry Pi Zero Legacy Version

The original Raspberry Pi Zero version was a working final/submission milestone, not a discarded or simulated port. Its final state is preserved on `legacy/pi-zero-final` and in the existing `v1.0-final-submission` tag.

## Build

The build requires GNU Make plus the AArch64 bare-metal-capable GNU cross tools available as:

- `aarch64-linux-gnu-gcc`
- `aarch64-linux-gnu-objcopy`

On Windows, run the build in WSL or another Linux environment with that toolchain installed. From the repository root:

```sh
make clean
make
```

The resulting Raspberry Pi firmware image is:

```text
build/kernel8.img
```

This checkpoint has no automated `make test` target.

## Project Structure

- `boot/boot.S` — AArch64 entry point and early processor setup
- `kernel/` — framebuffer, mailbox, terminal, shell, input, and in-memory demo storage code
- `linker.ld` — kernel memory layout
- `Makefile` — cross-compilation and image generation
- `hello.txt`, `notes.txt`, `resume.txt` — source content embedded in the demo storage image

## Branches / Versions

- `main` — stable Raspberry Pi 5 version based on known-good checkpoint `25e238c`
- `legacy/pi-zero-final` — original working Raspberry Pi Zero final/submission version
- `pi5-usb-keyboard` — experimental Raspberry Pi 5 USB/xHCI development; not fully hardware validated
- `experimental/pi5-usb-keyboard` — preservation alias for the experimental branch checkpoint
- `safety/main-before-pi5-promotion` — the previous `main` tip retained before promotion

## Roadmap

- continue Raspberry Pi 5 hardware support
- complete native USB/xHCI enumeration
- add hardware-backed HID keyboard input
- develop additional low-level drivers

## Author

Vedant Bhagat
