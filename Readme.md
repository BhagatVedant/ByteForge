# ByteForge

**ByteForge** is a custom bare-metal operating system project for the Raspberry Pi Zero, written from scratch in **ARM assembly** and **C**.

The goal of ByteForge is to explore low-level operating system development by building directly on hardware without relying on a traditional OS kernel. The long-term vision is to turn the Raspberry Pi into a lightweight personal file hub with a minimal command-based interface.

## Current Progress

At the moment, ByteForge includes:

- ARM boot entry code
- C kernel entry point
- custom linker script
- mailbox communication with the Raspberry Pi GPU
- framebuffer initialization
- basic pixel and rectangle drawing
- Makefile-based build system that generates a bootable `kernel.img`

This means the project has already moved beyond planning and now builds into a real bare-metal kernel image.

## Project Structure

```text
ByteForge/
├── boot/
│   └── boot.S
├── kernel/
│   ├── kernel.c
│   ├── mailbox.c
│   ├── mailbox.h
│   ├── framebuffer.c
│   └── framebuffer.h
├── build/
├── linker.ld
├── Makefile
└── README.md