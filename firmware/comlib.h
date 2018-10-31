#ifndef COMLIB_H
#define COMLIB_H

#include <stdio.h>
#include <string.h>
#include "usb.h"

#define COM_ENDPOINT 2

static inline void send_string_sync(uint8_t endpoint, const char *str);
static inline bool usb_ready();

inline void enable_echo();
inline void disable_echo();

unsigned char * com_readline();
void com_print(const char * str);
void com_println(const char * str);

//xc18 can't handle 
//#define com_printfln(s, ...) printf(s "\r\n", __VA_ARGS__)

extern unsigned comblib_drops;

#endif
