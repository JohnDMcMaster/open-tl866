/* 
 * File:   io.h
 * Author: William
 *
 * Created on April 16, 2018, 7:05 PM
 */

#ifndef IO_H
#define	IO_H

#ifdef	__cplusplus
extern "C" {
#endif


typedef unsigned char zif_bits_t[5];
typedef unsigned char port_bits_t[8];
typedef unsigned char latch_bits_t[8];

typedef struct port_info {
    volatile unsigned char * addr;
    unsigned char offset;
} port_info_t;

typedef struct latch_info {
    int number; /* Translates to LE signal write within case statement in write_latch() */
    int offset; /* Offset within the latch of the current bit.
                 *  -1 reserved for "no connection". */
} latch_info_t;

#define PORT_ADDR_TO_ARRAY_INDEX(_x)

#ifdef	__cplusplus
}
#endif

#endif	/* IO_H */

