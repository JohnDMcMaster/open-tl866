#ifndef AT89_H
#define AT89_H

#include <xc.h>
#include <string.h>
#include "usb.h"
#include "../../../system.h"
#include "../../../comlib.h"
#include "../../../parse.h"

zif_bits_t zbits_null = { 0 };


int programmer_at89(void);

#endif
