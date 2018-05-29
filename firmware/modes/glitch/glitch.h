/* 
 * open-tl866 firmware -- Glitch mode
 */

#ifndef GLITCH_H
#define GLITCH_H

#include <xc.h>
#include <string.h>
#include "../../comlib.h"
#include "../../io.h"

// TODO this needs to get fleshed out.
typedef struct trigger {
    zif_bits_t clock_pin;
    unsigned int glitch_width;
    unsigned int glitch_offset;
} trigger_t;

int glitch(void);

#endif
