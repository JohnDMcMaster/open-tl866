#ifndef AT89_H
#define AT89_H

#include "io.h"

unsigned char at89_read(unsigned int addr);
void at89_write(unsigned int addr, unsigned char data);
void at89_erase();
void at89_lock(unsigned char mode);
unsigned char at89_read_sysflash(unsigned int offset);
unsigned char at89_read_sig(unsigned int offset);

// Flip clock pin directly from TL866
#define at89_pin_flip_clock()                                                  \
    do {                                                                       \
        PORTE ^= 0x4;                                                          \
    } while (0)

extern zif_bits_t at89_zbits_null;
extern zif_bits_t at89_gnd;
extern zif_bits_t at89_vdd;
extern zif_bits_t at89_vpp;

#endif
