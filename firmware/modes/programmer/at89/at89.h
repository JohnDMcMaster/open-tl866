#ifndef AT89_H
#define AT89_H

#include <xc.h>
#include <string.h>
#include <setjmp.h>
#include "usb.h"
#include "../../../system.h"
#include "../../../comlib.h"
#include "../../../parse.h"


#define ZIFMASK_XTAL1 4;
#define ZIFMASK_GND 8;
#define ZIFMASK_VDD 128;
#define ZIFMASK_VPP 64;
#define ZIFMASK_PROG 32;

int programmer_at89(void);
void exit_glitch(void);

#endif
