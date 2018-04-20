/* 
 * File:   parse.h
 * Author: William
 *
 * Created on April 19, 2018, 5:24 PM
 */

#ifndef PARSE_H
#define	PARSE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "io.h"
  
typedef union tl866_bits {
    unsigned char voltage;
    zif_bits_t zif;
    port_bits_t port;
    latch_bits_t latch;
} tl866_bits_t;

typedef enum cmd {
    WRITE_ZIF,
    READ_ZIF,
    CONTINUE, // Not used right now, meant to represent "need more input"
    INVALID
} cmd_t;

typedef struct parse_result {
    cmd_t cmd;
    tl866_bits_t arg;
} parse_result_t;

void parse_ascii(unsigned char * buf, parse_result_t * res);



#ifdef	__cplusplus
}
#endif

#endif	/* PARSE_H */

