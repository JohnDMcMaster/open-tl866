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
typedef unsigned char port_bits_t[9]; /* PORTs A B C D E F G H J */
typedef unsigned char latch_bits_t[8];

typedef struct port_info {
    unsigned char bank;
    unsigned char offset;
} port_info_t;

typedef struct latch_info {
    unsigned char number; /* Translates to LE signal write within case statement in write_latch() */
    signed char offset; /* Offset within the latch of the current bit.
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

#define MYSTERY PORTBbits.RB1

// Prototypes
void dir_write(zif_bits_t zif_val);
void zif_write(zif_bits_t zif_val);
void zif_read(zif_bits_t zif_val);
void write_latch(int latch_no, unsigned char val);
void write_shreg(unsigned char in);
void vpp_en(void);
void vpp_dis(void);
void vdd_en(void);
void vdd_dis(void);
void set_vpp(zif_bits_t zif);
void set_vdd(zif_bits_t zif);
void set_gnd(zif_bits_t zif);
void vpp_val(unsigned char setting);
void vdd_val(unsigned char setting);


#ifdef	__cplusplus
}
#endif

#endif	/* IO_H */

