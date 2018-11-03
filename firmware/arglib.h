#ifndef ARGLIB_H
#define ARGLIB_H

#include <stdio.h>
#include <string.h>
#include "io.h"

/*
use strtok() to parse the next argument as int16_t

Return 1 on parse success, 0 otherwise
Set last_i on success
*/
int arg_i(void);

/*
Return 1 on parse success, 0 otherwise
Set last_bit on success
*/
int arg_bit(void);

/*
Given an argument like
0123456789
Parse first byte as 0x01, second as 0x23, etc

Return 1 on parse success, 0 otherwise
Set last_zif on success
*/
int arg_zif(void);

/*
Convert hex nibble to int
Return -1 on error
*/
int hex_c2i(char c);

extern unsigned comblib_drops;
extern int last_i;
extern int last_bit;
extern zif_bits_t last_zif;

#endif
