/*
 * open-tl866 firmware
 * TODO: Add description
 */

#ifndef BITBANG_H
#define	BITBANG_H

#include <xc.h>
#include <string.h>
#include "../../comlib.h"
#include "../../parse.h"

static inline void handle_command(parse_result_t *res);
int bitbang(void);

#endif