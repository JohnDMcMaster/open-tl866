
#include <xc.h>
#include <usb.h>
#include <usb_ch9.h>

#include "stock_compat.h"
#include "comlib.h"



struct serial_block_t serial_block = {
    {0}, // dev_code
    {    sizeof(((struct serial_block_t *)0)->serial),
        DESC_STRING,
        {0}
    } // serial
};

void stock_load_serial_block()
{
    uint8_t block[80];
    uint8_t carry, hold;
    int idx;

    TBLPTR = 0x1FD00;
    for (idx = 0; idx < 80; idx++) {
        asm("tblrd*+");
        block[idx] = TABLAT;
    }

    // xor the block with the keystream
    TBLPTR = 0x1FC00 + 0x0A;
    for (idx = 0; idx < 80; idx++) {
        asm("tblrd*+");
        block[idx] ^= TABLAT;
    }

    // shift the entire block right by three bits
    carry = 0;
    for (idx = 0; idx < 80; idx++) {
        hold = block[idx] << 5;
        block[idx] = (block[idx] >> 3) | carry;
        carry = hold;
    }

    // swap bytes around
    for (idx = 0; idx < 40; idx += 4) {
        carry = 80 - idx - 1;
        hold = block[idx];
        block[idx] = block[carry];
        block[carry] = hold;
    }


    // copy out device code field
    for (idx = 0; idx < 8; idx++) {
        serial_block.dev_code[idx] = block[idx];
    }

    // copy out serial number field
    // these are ISO 8859-1 codepoints, so they're all valid UTF-16
    for (idx = 0; idx < 24; idx++) {
        serial_block.serial.chars[idx] = block[idx + 8];
    }
}



#define CMD_REPORT  0x00
#define CMD_RESET   0xFF

#define STATUS_NORMAL      1
#define STATUS_BOOTLOADER  2

#define MODEL_TL866A   1
#define MODEL_TL866CS  2

struct report_t {
    uint8_t echo;
    uint8_t status;
    uint16_t report_size;
    uint8_t version_minor;
    uint8_t version_major;
    uint8_t model;
    uint8_t device_code[8];
    uint8_t serial_number[24];
    uint8_t version_hardware;
};

static void send_report()
{
    struct report_t *report;
    int idx;

    report = (struct report_t *) usb_get_in_buffer(STOCK_ENDPOINT);

    report->echo = CMD_REPORT;
    report->status = STATUS_NORMAL;
    report->report_size = sizeof(*report);

    // FIXME: figure out how to detect the model properly
    report->model = MODEL_TL866CS;

    report->version_hardware = 255;
    report->version_major = 1;
    report->version_minor = 0;

    for (idx = 0; idx < 8; idx++) {
        report->device_code[idx] = serial_block.dev_code[idx];
    }

    for (idx = 0; idx < 24; idx++) {
        report->serial_number[idx] = serial_block.serial.chars[idx];
    }

    usb_send_in_buffer(STOCK_ENDPOINT, sizeof(*report));
}

void stock_disable_usb()
{
    uint8_t cycles;

    // disable the USB module
    UCONbits.USBEN = 0;

    // wait... some time
    // this is copied from the stock firmware
    INTCONbits.TMR0IE = 0;
    T0CON = 0x84;
    for (cycles = 0x0F; cycles > 0; cycles--) {
        TMR0 = 0;
        INTCONbits.TMR0IF = 0;
        while (!INTCONbits.TMR0IF);
    }
    T0CON = 0;
}

void stock_reset_to_bootloader()
{
    stock_disable_usb();

    // set the bootloader signature and reboot
    *((uint32_t*)0x0700) = 0x55AA55AA;
    RESET();
}


void stock_handle_out_txn()
{
    const char *buffer;
    uint8_t size;

    size = usb_get_out_buffer(STOCK_ENDPOINT, &buffer);

    switch (buffer[0]) {
    case CMD_REPORT:
        send_report();
        break;

    case CMD_RESET:
        stock_reset_to_bootloader();
        break;
    }

    usb_arm_out_endpoint(STOCK_ENDPOINT);
}
