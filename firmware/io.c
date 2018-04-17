#include <xc.h>

#include "io.h"

static void port_read_all(port_bits_t *);
static void port_write_all(port_bits_t *);

/* ZIF pin assignments are scattered all over the PIC18's I/O banks.
 * This array provides a mapping from a ZIF pin to the corresponding PIC18
 * I/O location.
 * __bit data type exists, but can't take address of it,
 *  so use char data type. */
const port_info_t zif2port[2] = {
    {&PORTC, 5},
    {&PORTC, 4},
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

/* Currently I don't know whether it's faster to just eat the full port
 * read every time we want to set/clear current bits, or programmatically
 * determine a sequence of |=/&= that can be optimized to bitset/clr insns.
 
 A mask at one at a given location does the action:
 0 : Clear
 1 : Set
 2 : Toggle
 Else : Read bits are echoed back to ports. */
void zif_modify(zif_bits_t * zif_val, const int action)
{
    port_bits_t port_val;
    
    port_read_all(&port_val);
    
    int pin_no = 0;
    
    for(unsigned int i = 0; i < 5; i++)
    {
        for(unsigned int j = 0; j < 8; j++)
        {
            unsigned char mask = 1u << j;
            
            if((* zif_val)[i] & mask)
            {
                port_info_t curr = zif2port[pin_no];
                
                switch(action)
                {
                    case 0:
                        break;
                        
                    case 1:
                        break;
                        
                    case 2:
                        break;
                        
                    default:
                        break;
                }
            }
            
            pin_no++;
        }
    }
    
    port_write_all(&port_val);
}


/* Internal functions- we read/write all I/O ports at once. */
static void port_read_all(port_bits_t * p_bits)
{
    (* p_bits)[0] = PORTA;
    (* p_bits)[1] = PORTB;
    (* p_bits)[2] = PORTC;
    (* p_bits)[3] = PORTD;
    (* p_bits)[4] = PORTE;
    (* p_bits)[5] = PORTF;
    (* p_bits)[6] = PORTG;
    (* p_bits)[7] = PORTH;
}

static void port_write_all(port_bits_t * p_bits)
{
    PORTA = (* p_bits)[0];
    PORTB = (* p_bits)[1];
    PORTC = (* p_bits)[2];
    PORTD = (* p_bits)[3];
    PORTE = (* p_bits)[4];
    PORTF = (* p_bits)[5];
    PORTG = (* p_bits)[6];
    PORTH = (* p_bits)[7];
}



/* Read mask of I/O pin dir (TRIS) into array in ZIF order. */
void zifdir_mask(unsigned char (*)[5])
{
    
}


/* Write one of the 8 pin driver latches */
void write_latch(int, unsigned char)
{
    
}

/* Write the shift reg which connects to pin driver latches */
void write_shreg(unsigned char)
{
    
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