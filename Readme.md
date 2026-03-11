# ByteForge

ByteForge is a custom bare-metal operating system project for the Raspberry Pi Zero, written from scratch in ARM assembly and C.

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

This means the project already builds into a real bare-metal kernel image.

## Project Structure

ByteForge/
│
├── boot/
│   └── boot.S
│
├── kernel/
│   ├── kernel.c
│   ├── mailbox.c
│   ├── mailbox.h
│   ├── framebuffer.c
│   └── framebuffer.h
│
├── build/
├── linker.ld
├── Makefile
└── README.md

## How It Works

On boot, the Raspberry Pi firmware loads `kernel.img` from the SD card. ByteForge then takes control and runs its own startup code.

Boot flow:

Raspberry Pi firmware  
→ ByteForge boot assembly  
→ C kernel  
→ mailbox request  
→ framebuffer setup  
→ basic graphics drawing  

## Build Requirements

This project is built in WSL / Linux using the ARM bare-metal toolchain.

Required tools:

- arm-none-eabi-gcc
- make

Install on Ubuntu / WSL with:

sudo apt update
sudo apt install gcc-arm-none-eabi make

## Building

From the project root, run:

make clean
make

This produces:

build/kernel.img

which is the kernel image intended to be loaded by the Raspberry Pi boot process.

## Current Target Hardware

- Raspberry Pi Zero
- HDMI output
- microSD boot media

## Roadmap

Planned next steps include:

- booting ByteForge directly on hardware
- rendering text to the screen
- building a simple terminal-style interface
- adding keyboard input
- implementing basic file and storage commands
- evolving ByteForge into a minimal personal file hub

## Why This Project Exists

ByteForge is being built as an Operating Systems final project, but the idea goes beyond the class itself. The aim is to create something technically challenging, practical, and portfolio-worthy while learning how operating systems actually work at a low level.

## Author

Vedant Bhagat