# ByteForge USB Attempt (From Scratch)

This branch is me trying to get USB working from scratch on the Raspberry Pi Zero.

Goal was to:
- get keyboard input directly into my OS
- later use USB for storage (like real files instead of hardcoded ones)

## What I got working

- initialized USB controller (DWC2) in host mode
- powered + reset the USB port
- detected a device on the root port (in my case, the hub)
- set up control transfer structures (EP0, setup packet, etc.)
- tried both DMA and non-DMA (FIFO) paths

## What didn’t work

- control transfers never actually executed
- `HCINT0` always stayed 0
- no descriptor data came back (all zeros)
- couldn’t move past the first step of enumeration

Basically:
> the controller sees the device, but I can’t actually talk to it yet

## Important thing I learned

My setup was:

Pi → hub → keyboard

So the Pi was actually talking to the **hub**, not the keyboard.

To get the keyboard working from scratch I would need to:
- fully enumerate the hub
- power/reset hub ports
- then enumerate the keyboard behind it

Which is way more work than I expected.

## Why I’m stopping here (for now)

I got far enough to understand how USB host + control transfers work at a low level.

But finishing:
- hub support
- HID keyboard
- storage

would take a lot more time.

So I’m switching to trying a library next to actually get input working.

## Next step

New branch:
- test USB using a library (USPi or similar)
- goal is just to get keyboard input working first

I might come back to this later.