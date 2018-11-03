#ifndef _STOCK_COMPAT_H
#define _STOCK_COMPAT_H

#include <stdint.h>

#define STOCK_ENDPOINT 1

extern struct serial_block_t {
    uint8_t dev_code[8];

    // this is a USB string descriptor structure
    // so we don't need two copies to use it in the USB descriptor
    struct {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint16_t chars[24];
    } serial;
} serial_block;

void stock_load_serial_block();

void stock_disable_usb();
void stock_reset_to_bootloader();

#endif
