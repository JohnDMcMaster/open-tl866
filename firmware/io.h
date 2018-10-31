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

/*
40 pins represented as 5 bytes => 40 bits
first byte, LSB is pin 1?
*/
typedef unsigned char zif_bits_t[5];
typedef const unsigned char const_zif_bits_t[5];
//PIC MCU pins grouped by access register A-J
typedef unsigned char port_bits_t[9]; /* PORTs A B C D E F G H J */
//Actual latch bits
typedef unsigned char latch_bits_t[8];
//For debugging only
extern latch_bits_t latch_cache;


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
/*
Set bit to 1 to make output
Default: all inputs
*/
void dir_write(zif_bits_t zif_val);
void dir_read(zif_bits_t zif_val);
void zif_write(zif_bits_t zif_val);
void zif_read(zif_bits_t zif_val);
void write_latch(int latch_no, unsigned char val);
void write_shreg(unsigned char in);


//(VPP_98, VPP_126, VPP_140, VPP_166, VPP_144, VPP_171, VPP_185, VPP_212) = range(8)
//# My measurements: 9.83, 12.57, 14.00, 16.68, 14.46, 17.17, 18.56, 21.2
#define VPP_98 0
#define VPP_126 1
#define VPP_140 2
#define VPP_166 3
#define VPP_144 4
#define VPP_171 5
#define VPP_185 6
#define VPP_212 7
void vpp_val(unsigned char setting);
void set_vpp(const_zif_bits_t zif);
void vpp_en(void);
void vpp_dis(void);
int vpp_state(void);

//(VDD_30, VDD_35, VDD_46, VDD_51, VDD_43, VDD_48, VDD_60, VDD_65) = range(8)
//# My measurements: 2.99, 3.50, 4.64, 5.15, 4.36, 4.86, 6.01, 6.52
#define VDD_30 0
#define VDD_35 1
#define VDD_46 2
#define VDD_51 3
#define VDD_43 4
#define VDD_48 5
#define VDD_60 6
#define VDD_65 7
void vdd_val(unsigned char setting);
void set_vdd(const_zif_bits_t zif);
void vdd_en(void);
void vdd_dis(void);
int vdd_state(void);

void set_gnd(const_zif_bits_t zif);

//Low level API
//MCU
//Write MCU pin output values
//You must also set tristate (dir_write_all()) if you want to actually drive out
void port_write_all(port_bits_t p_bits);
//Read MCU pins
void port_read_all(port_bits_t p_bits);
//Set MCU tristate. 1 => output
void dir_write_all(port_bits_t p_bits);
//Read tristate setting
void dir_read_all(port_bits_t p_bits);
//This should always return 0x00 (no latches enabled)
int OEn_state(void);



#ifdef	__cplusplus
}
#endif

#endif	/* IO_H */
