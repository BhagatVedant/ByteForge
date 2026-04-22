# ByteForge OS

ByteForge is a bare-metal operating system I built from scratch for the Raspberry Pi Zero.

The main idea behind it is to turn the Pi into a simple personal storage OS, kind of like a very early version of a DIY Google Drive.

Right now it is still a prototype, but the core parts are already working.

## What it currently does

- Boots directly on the Raspberry Pi with no Linux underneath
- Sets up a framebuffer and displays text on screen
- Has a custom terminal and shell
- Has a basic storage system with commands like:
  - `devices`
  - `mount usb0`
  - `ls`
  - `open <file>`
  - `cat <file>`
  - `unmount`
- Handles errors, like trying to open a file that does not exist
- Supports multiple files loaded into the system

The file content comes from external `.txt` files that I convert and load into the OS, so it is not just random hardcoded strings in the shell output.

## How it works

The OS is built in layers:

boot -> framebuffer -> terminal -> input -> shell -> storage

- `boot` starts everything
- `framebuffer` lets me draw to the screen
- `terminal` handles text output and the cursor
- `shell` processes commands
- `storage` simulates a file system and external device workflow

## Example flow

storage@byteforge > devices
storage@byteforge > mount usb0
storage@byteforge > ls
storage@byteforge > open hello.txt
storage@byteforge > cat resume.txt
storage@byteforge > unmount

## Why I built this

I did not want to just make a normal shell or app that runs on top of an operating system.

I wanted to understand how things work underneath, starting from booting the machine all the way to interacting with files inside my own OS.

The long-term idea is to turn this into a small personal file server where you can plug in storage and access files directly.

## What is next

The current version is a working prototype.

Next steps would be things like:
- real keyboard input
- actual USB storage access instead of simulated files
- eventually some way to move files in from another device

## Build

From the project root, run:

make

Then copy the generated `kernel.img` to the Raspberry Pi boot partition and run it.

## Notes

This is still a work in progress, but the core system is stable and runs directly on real hardware.

## Author

Vedant Bhagat