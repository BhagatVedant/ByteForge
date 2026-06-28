#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdint.h>

#define MBOX_CHANNEL_PROPERTY 8

int mailbox_call(uint8_t channel);

#endif
