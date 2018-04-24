#include <xc.h>

#include "system.h"
#include "io.h"

static void ports_to_zif_pins(port_bits_t port, zif_bits_t zif);
static void zif_pins_to_ports(zif_bits_t zif, port_bits_t port);
static void port_read_all(port_bits_t);
static void port_write_all(port_bits_t);
static void dir_write_all(port_bits_t p_bits);

#define OFFS_A 0
#define OFFS_B 1
#define OFFS_C 2
#define OFFS_D 3
#define OFFS_E 4
#define OFFS_F 5
#define OFFS_G 6
#define OFFS_H 7
#define OFFS_J 8

/* ZIF pin assignments are scattered all over the PIC18's I/O banks.
 * This array provides a mapping from a ZIF pin to the corresponding PIC18
 * I/O location (0-based port bank addressing, and bit offset). */
const port_info_t zif2port[40] = {
    {OFFS_C, 5},
    {OFFS_C, 4},
    {OFFS_C, 3},
    {OFFS_C, 2},
    {OFFS_J, 7},
    {OFFS_J, 6},
    {OFFS_C, 6},
    {OFFS_C, 7},
    
    {OFFS_J, 4},
    {OFFS_J, 5},
    {OFFS_G, 3},
    {OFFS_G, 2},
    {OFFS_D, 0},
    {OFFS_D, 1},
    {OFFS_D, 2},
    {OFFS_G, 1},
    
    {OFFS_E, 0},
    {OFFS_E, 7},
    {OFFS_E, 2},
    {OFFS_E, 3},
    {OFFS_E, 4},
    {OFFS_E, 5},
    {OFFS_E, 6},
    {OFFS_E, 1},
    
    {OFFS_D, 3},
    {OFFS_D, 4},
    {OFFS_D, 5},
    {OFFS_D, 6},
    {OFFS_D, 7},
    {OFFS_G, 0},
    {OFFS_J, 0},
    {OFFS_J, 1},
    
    {OFFS_J, 2},
    {OFFS_J, 3},
    {OFFS_B, 2},
    {OFFS_B, 3},
    {OFFS_B, 4},
    {OFFS_B, 5},
    {OFFS_B, 6},
    {OFFS_B, 7},
};

/* LE signal number is based off radioman schematic. They are scattered
 * between banks unfortunately. */
const latch_info_t zif2vdd[2] = {
    {3, 5},
    {3, 6},
    
};

const latch_info_t zif2vpp[40] = {
    {0, 2},
    {0, 3},
    {1, 2},
    {1, 3},
    
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},
    
    {1, 5},
    {1, 4},
    {0, -1},
    {0, -1},
    
    {0, -1},   
    {0, -1},
    {0, -1},
    {0, -1},
    
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},
    
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},
    
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},
    
    {0, -1},
    {0, 0},
    {1, 0},
    {0, 7},
    
    {1, 6},
    {1, 1},
    {0, -1},
    {0, 1},
    
    {1, 7},
    {0, 6},
    {0, 5},
    {0, 4},
};

const latch_info_t zif2gnd[2] = {
    {6, 0},
    {6, 1},
};

static unsigned char latch_mirror[8]; /* Read mirror of the current latch state. */


void dir_write(zif_bits_t zif_val)
{
    port_bits_t port_val = {0};
    
    zif_pins_to_ports(zif_val, port_val);
    
    dir_write_all(port_val);
}


void zif_write(zif_bits_t zif_val)
{
    port_bits_t port_val = {0};
    
    zif_pins_to_ports(zif_val, port_val);
    
    port_write_all(port_val);
}

void zif_read(zif_bits_t zif_val)
{
    port_bits_t port_val = {0};
    
    port_read_all(port_val);
    ports_to_zif_pins(port_val, zif_val);
}

static void zif_pins_to_ports(zif_bits_t zif, port_bits_t port)
{
    for(unsigned int pin_no = 0; pin_no < (sizeof(zif2port)/sizeof(port_info_t)); pin_no++)
    {
        /* It would be nice to assume the compiler can divide by 8 using
        shifts, but XC8 is broken in free mode. */
        unsigned char set_of_8 = (pin_no >> 3);
        unsigned char bit_offset = 1 << (pin_no & 0x07);
        
        unsigned char pin_val = (zif[set_of_8] & bit_offset) ? 1 : 0;
        
        port_info_t curr = zif2port[pin_no];      
        port[curr.bank] |= (pin_val << curr.offset);
    } 
}


static void ports_to_zif_pins(port_bits_t port, zif_bits_t zif)
{
    for(unsigned int pin_no = 0; pin_no < (sizeof(zif2port)/sizeof(port_info_t)); pin_no++)
    {
        port_info_t curr = zif2port[pin_no];

        unsigned char set_of_8 = (pin_no >> 3);
        unsigned char bit_offset = (pin_no & 0x07);
        unsigned char port_offset = (1 << curr.offset);
        
        unsigned char pin_val = (port[curr.bank] & port_offset) ? 1 : 0;
   
        zif[set_of_8] |= (pin_val << bit_offset);
    }
}


/* Internal functions- we read/write all I/O ports at once. */
static void port_read_all(port_bits_t p_bits)
{
    p_bits[0] = PORTA;
    p_bits[1] = PORTB;
    p_bits[2] = PORTC;
    p_bits[3] = PORTD;
    p_bits[4] = PORTE;
    p_bits[5] = PORTF;
    p_bits[6] = PORTG;
    p_bits[7] = PORTH;
    p_bits[8] = PORTJ;
}

static void port_write_all(port_bits_t p_bits)
{
    PORTA = p_bits[0];
    PORTB = p_bits[1];
    PORTC = p_bits[2];
    PORTD = p_bits[3];
    PORTE = p_bits[4];
    PORTF = p_bits[5];
    PORTG = p_bits[6];
    PORTH = p_bits[7];
    PORTJ = p_bits[8];
}

static void dir_write_all(port_bits_t p_bits)
{
    TRISA = p_bits[0];
    TRISB = p_bits[1];
    TRISC = p_bits[2];
    TRISD = p_bits[3];
    TRISE = p_bits[4];
    TRISF = p_bits[5];
    TRISG = p_bits[6];
    TRISH = p_bits[7];
    TRISJ = p_bits[8];
}


/* Read mask of I/O pin dir (TRIS) into array in ZIF order. */
void zifdir_mask(unsigned char (*)[5])
{
    
}


/* Write one of the 8 pin driver latches */
void write_latch(int latch_no, unsigned char val)
{
    write_shreg(val);
    
    //74hc373's setup time: 15ns hold time: 5ns
    //LE == 0 to preserve output, 1 is "transparent mode".
    
    switch(latch_no)
    {
        case 0:
            LE0 = 1;
            __delay_us(1);
            LE0 = 0;
            __delay_us(1);
            break;
        case 1:
            LE1 = 1;
            __delay_us(1);
            LE1 = 0;
            __delay_us(1);
            break;
        case 2:
            LE2 = 1;
            __delay_us(1);
            LE2 = 0;
            __delay_us(1);
            break;
        case 3:
            LE3 = 1;
            __delay_us(1);
            LE3 = 0;
            __delay_us(1);
            break;
        case 4:
            LE4 = 1;
            __delay_us(1);
            LE4 = 0;
            __delay_us(1);
            break;
        case 5:
            LE5 = 1;
            __delay_us(1);
            LE5 = 0;
            __delay_us(1);
            break;
        case 6:
            LE6 = 1;
            __delay_us(1);
            LE6 = 0;
            __delay_us(1);
            break;
        case 7:
            LE7 = 1;
            __delay_us(1);
            LE7 = 0;
            __delay_us(1);
            break;
        default:
            break;
        
    }
}

/* Write the shift reg which connects to pin driver latches */
void write_shreg(unsigned char in)
{
    for(int i = 0; i < 8; i++)
    {
        unsigned char curr_bit = (in & 0x80) ? 1 : 0;
        
        SR_DAT = curr_bit;
        // 74hc164's data setup time: 20-100ns Hold time: 5ns
        __delay_us(1);
        SR_CLK = 1;
        __delay_us(1);
        SR_CLK = 0;
        
        in = in << 1;
    }
}

/* Read mirror of one of the 8 pin driver latches */
/* unsigned char read_latch(int)
{
    
} */

void vpp_en(void)
{
    OE_VPP = 0;
}

void vpp_dis(void)
{
    OE_VPP = 1;
}

void vdd_en(void)
{
    OE_VDD = 0;
}

void vdd_dis(void)
{
    OE_VDD = 1;
}


void set_vpp(zif_bits_t zif)
{
    unsigned char latch_vpp_masks[2] = { 0 };
    
    for(unsigned int pin_no = 0; pin_no < (sizeof(zif2vpp)/sizeof(latch_info_t)); pin_no++)
    {
        latch_info_t curr = zif2vpp[pin_no];

        // Commands are zif-based; don't bother assigning a voltage to a zif
        // pin which doesn't have any (offset == -1 or less).
        if(curr.offset < 0)
        {
            continue;
        }
        else
        {
            unsigned char set_of_8 = (pin_no >> 3);
            unsigned char bit_offset = 1 << (pin_no & 0x07);
        
            unsigned char mask = (zif[set_of_8] & bit_offset) ? 1 : 0;
      
            latch_vpp_masks[curr.number] |= (mask << (unsigned char) curr.offset);
        }
    }
    
    write_latch(0, latch_vpp_masks[0]);
    write_latch(1, latch_vpp_masks[1]);
}

void set_vdd(zif_bits_t zif)
{
    unsigned char latch_vdd_masks[3] = { 0 };
    
    for(unsigned int pin_no = 0; pin_no < (sizeof(zif2vdd)/sizeof(latch_info_t)); pin_no++)
    {
        latch_info_t curr = zif2vdd[pin_no];

        if(curr.offset < 0)
        {
            continue;
        }
        else
        {
            unsigned char set_of_8 = (pin_no >> 3);
            unsigned char bit_offset = 1 << (pin_no & 0x07);
        
            // Drivers are PNPs, so a logic 0 enables Vdd line for I/O pin.
            unsigned char mask = (zif[set_of_8] & bit_offset) ? 1 : 0;
      
            latch_vdd_masks[curr.number - 2] |= (mask << (unsigned char) curr.offset);
        }
    }
    
    write_latch(2, latch_vdd_masks[0]);
    write_latch(3, latch_vdd_masks[1]);
    write_latch(4, latch_vdd_masks[2]);
}

void set_gnd(zif_bits_t zif)
{
    unsigned char latch_gnd_masks[3] = { 0 };
    
    for(unsigned int pin_no = 0; pin_no < (sizeof(zif2gnd)/sizeof(latch_info_t)); pin_no++)
    {
        latch_info_t curr = zif2gnd[pin_no];

        if(curr.offset < 0)
        {
            continue;
        }
        else
        {
            unsigned char set_of_8 = (pin_no >> 3);
            unsigned char bit_offset = 1 << (pin_no & 0x07);
        
            // Drivers are PNPs, so a logic 0 enables Vdd line for I/O pin.
            unsigned char mask = (zif[set_of_8] & bit_offset) ? 1 : 0;
      
            latch_gnd_masks[curr.number - 5] |= (mask << (unsigned char) curr.offset);
        }
    }
    
    write_latch(5, latch_gnd_masks[0]);
    write_latch(6, latch_gnd_masks[1]);
    write_latch(7, latch_gnd_masks[2]);
}

void vpp_val(unsigned char setting)
{
    setting &= 0x07;
    
    VID_10 = (setting & 0x01) ? 1 : 0;
    VID_11 = (setting & 0x02) ? 1 : 0;
    VID_12 = (setting & 0x04) ? 1 : 0;
    
    __delay_ms(2);
}

void vdd_val(unsigned char setting)
{
    setting &= 0x07;
    
    VID_00 = (setting & 0x01) ? 1 : 0;
    VID_01 = (setting & 0x02) ? 1 : 0;
    VID_02 = (setting & 0x04) ? 1 : 0;
    
    __delay_ms(2);
}