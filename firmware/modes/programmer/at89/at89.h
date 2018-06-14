#ifndef AT89_H
#define AT89_H

#include <xc.h>
#include <string.h>
#include "usb.h"
#include "../../../system.h"
#include "../../../comlib.h"
#include "../../../parse.h"


#define ZIFMASK_XTAL1 4;
#define ZIFMASK_GND 8;
#define ZIFMASK_VDD 128;
#define ZIFMASK_VPP 64;
#define ZIFMASK_PROG 32;

zif_bits_t zbits_null = {0, 0, 0, 0, 0};
zif_bits_t gnd        = {0, 0, 8, 0, 0};
zif_bits_t vdd        = {0, 0, 0, 0, 128};
zif_bits_t vpp        = {0, 0, 0, 64, 0};

int programmer_at89(void);

#endif
