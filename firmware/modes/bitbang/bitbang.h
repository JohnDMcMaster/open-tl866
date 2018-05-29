/*
 * open-tl866 firmware
 * TODO: Add description
 */

#ifndef BITBANG_H
#define	BITBANG_H

#include <xc.h>
#include <string.h>
#include "usb.h"
#include "../../parse.h"

static void send_string_sync(uint8_t endpoint, const char *str);
static inline void handle_command(parse_result_t *res);
int bitbang(void);

#endif