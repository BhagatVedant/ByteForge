# ByteForge

ByteForge is a bare metal operating system for a Raspberry Pi Zero W V1.1.

The main idea is to build a base framework for turning the Raspberry into a server OS to be used as a personal Google Drive.

Right now it is a base for this project to be build upon.

The code still uses Pi firmware to load my kernel image. That is the only part not done from scratch.

## What it can do right now

- Boots directly on the Raspberry Pi bare-metal without any Linux base.
- Sets up a mailbox system to communicate between GPU and CPU.
- Sets up a framebuffer to display text and shapes on a screen.
- Has a custom terminal and shell with basic commands for the system.
- Has a basic storage system with commands that can help do the following actions:
    - File listing
    - File opening
    - Displaying File metadata
    - mount and unmount USB (Right now only detects connection between microUSB)
    - Error handling for missing files
- Pipeline built for keyboard input until USB integration.

## Setup

(For the purposes for this final project, SDcard.zip has the Pi firmware with kernel.img shown in class demo for easier testing for the instructor which shall be submitted with the other files. Please refer to that for personal testing.)

Clone the repo or unzip the project file.

Do keep in mind! This project uses the ARM bare-metal cross compiler. To run the following commands you need to be on Linux or using wsl to simulate.

Once in a linux environment run the following command:
```bash
make clean
make
```

In the build directory made copy the following file and paste it in the Pi firmware.

```bash
build/kernel.img
```

## Hardware Used

- Raspberry Pi Zero W V1.1
- microSD card
- HDMI monitor
- Laptop with WSL and VS Code
- ARM cross compiler
- USB Work and microSD reader

## Other Things To Note

For this project, I tried to add real USB support from scratch using DWC2 USB controller where I reached the following checkpoints:
- USB port reset
- Device detection
- Speed detection

I stopped at EP0 control transfer as it would send out a packet but the device would not respond back.

This version is a working prototype. The main goal was to build the core parts of a small OS myself: boot, framebuffer output, terminal, shell, and a storage-style interface.

## Author

Vedant Bhagat