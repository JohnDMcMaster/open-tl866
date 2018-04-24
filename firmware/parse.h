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
    unsigned char led;
    zif_bits_t zif;
    port_bits_t port;
    latch_bits_t latch;
} tl866_bits_t;

typedef enum cmd {
    ECHO_ON,
    ECHO_OFF,
    GND_WRITE,
    LED_ON,
    LED_OFF,
    LED_QUERY,
    VDD_SET,
    VDD_WRITE,
    VPP_SET,
    VPP_WRITE,
    ZIF_DIR,
    ZIF_WRITE,
    ZIF_READ,
    CONTINUE, // Not used right now, meant to represent "need more input"
    INVALID
} cmd_t;

typedef struct parse_result {
    cmd_t cmd;
    unsigned char arg[10];
} parse_result_t;

void parse_ascii(unsigned char * buf, parse_result_t * res);
void ascii_to_hex(unsigned char * dst, unsigned char * src, size_t dst_len, size_t src_len);
void hex_to_ascii(unsigned char * dst, unsigned char * src, size_t dst_len, size_t src_len);



#ifdef	__cplusplus
}
#endif

#endif	/* PARSE_H */

