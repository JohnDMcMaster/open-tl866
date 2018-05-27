#ifndef COMLIB_H
#define COMLIB_H

#include <string.h>
#include "usb.h"

static inline void send_string_sync(uint8_t endpoint, const char *str);
static inline bool usb_ready();

void enable_echo();
void disable_echo();

unsigned char * com_readline();
void com_print(const char * str);

#endif
