/*
    File: mailbox.c
 
    This file handles the Raspberry Pi's mailbox system

    Mailbox is how ARM CPU talks to the GPU/firmware
    In our OS currently we only use to set up the framebuffer to draw on the screen.

    This is also what is used to get hardware information and things like random numbers in the future.
*/

#include "mailbox.h"

#define VIDEOCORE_MBOX  0x2000B880

#define MBOX_READ       ((volatile uint32_t*)(VIDEOCORE_MBOX + 0x0))
#define MBOX_STATUS     ((volatile uint32_t*)(VIDEOCORE_MBOX + 0x18))
#define MBOX_WRITE      ((volatile uint32_t*)(VIDEOCORE_MBOX + 0x20))

#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

/*
    This is the array that is shared with the GPU to send messages and receive messages.
    It is aligned to 16 bytes as required by the mailbox protocol as the last 4 bits of the address are used to specify the channel number.
*/
volatile uint32_t mailbox[36] __attribute__((aligned(16)));

//Sends  a mailbox message to the channel and waits for the response.
int mailbox_call(uint8_t channel) {
    uint32_t addr = ((uint32_t)((uintptr_t)mailbox) & ~0xF) | (channel & 0xF);
    
    //Wait until mailbox is not full
    while (*MBOX_STATUS & MBOX_FULL) { }

    //Send the message to GPU by writing the address to the write register.
    *MBOX_WRITE = addr;



    while (1) {
        //Wait until there is a response in the read register
        while (*MBOX_STATUS & MBOX_EMPTY) { }

        //Check response to make sure it is our specific request.
        uint32_t response = *MBOX_READ;
        if (response == addr) {
            return mailbox[1] == MBOX_RESPONSE;
        }
    }
}