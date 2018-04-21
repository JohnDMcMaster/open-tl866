#include <xc.h>

#include "system.h"
#include "io.h"

static void port_read_all(port_bits_t);
static void port_write_all(port_bits_t);

/* ZIF pin assignments are scattered all over the PIC18's I/O banks.
 * This array provides a mapping from a ZIF pin to the corresponding PIC18
 * I/O location (0-based port bank addressing, and bit offset). */
const port_info_t zif2port[2] = {
    {2, 5},
    {2, 4},
};

/* LE signal number is based off radioman schematic. They are scattered
 * between banks unfortunately. */
const latch_info_t zif2vdd[2] = {
    {3, 5},
    {3, 6},
};

const latch_info_t zif2vpp[2] = {
    {0, 2},
    {0, 3},
};

const latch_info_t zif2gnd[2] = {
    {6, 0},
    {6, 1},
};

static unsigned char latch_mirror[8]; /* Read mirror of the current latch state. */


void zif_write(zif_bits_t zif_val)
{
    port_bits_t port_val = {0};
    
    for(unsigned int pin_no = 0; pin_no < 2; pin_no++)
    {
        /* It would be nice to assume the compiler can divide by 8 using
        shifts, but XC8 is broken in free mode. */
        unsigned char set_of_8 = (pin_no >> 3);
        unsigned char bit_offset = 1 << (pin_no & 0x07);
        
        unsigned char pin_val = (zif_val[set_of_8] & bit_offset) ? 1 : 0;
        
        port_info_t curr = zif2port[pin_no];      
        port_val[curr.bank] |= (pin_val << curr.offset);
    }
    
    port_write_all(port_val);
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

void set_vpp(unsigned char)
{

}

void set_vdd(unsigned char)
{

}

void set_iov(unsigned char)
{
    
}