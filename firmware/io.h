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

// Static pin defines (uses radioman's identifiers)
#define SR_CLK PORTHbits.RH3
#define SR_DAT PORTHbits.RH2

#define OE_VPP PORTGbits.RG4
#define OE_VDD PORTAbits.RA4

#define LE0 PORTHbits.RH0
#define LE1 PORTHbits.RH1
#define LE2 PORTAbits.RA2
#define LE3 PORTAbits.RA0
#define LE4 PORTAbits.RA5
#define LE5 PORTAbits.RA3
#define LE6 PORTHbits.RH4
#define LE7 PORTAbits.RA1

#define VID_00 PORTFbits.RF5
#define VID_01 PORTFbits.RF6
#define VID_02 PORTFbits.RF7
#define VID_10 PORTHbits.RH6
#define VID_11 PORTHbits.RH7
#define VID_12 PORTFbits.RF2

#define GND20 PORTCbits.RC1
#define OVC PORTBbits.RB0

#define LED PORTCbits.RC0

// Prototypes
void write_latch(int latch_no, unsigned char val);
void write_shreg(unsigned char in);


#ifdef	__cplusplus
}
#endif

#endif	/* IO_H */

