//#define DEBUG(x) x
#define DEBUG(x) do {} while(0)

#include <xc.h>

#include "system.h"
#include "io.h"
#include "comlib.h"

latch_bits_t latch_cache = {0};

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
    //1-8
    {OFFS_C, 5},
    {OFFS_C, 4},
    {OFFS_C, 3},
    {OFFS_C, 2},
    {OFFS_J, 7},
    {OFFS_J, 6},
    {OFFS_C, 6},
    {OFFS_C, 7},

    //9-16
    {OFFS_J, 4},
    {OFFS_J, 5},
    {OFFS_G, 3},
    {OFFS_G, 2},
    {OFFS_D, 0},
    {OFFS_D, 1},
    {OFFS_D, 2},
    {OFFS_G, 1},

    //17-24
    {OFFS_E, 0},
    {OFFS_E, 7},
    {OFFS_E, 2},
    {OFFS_E, 3},
    {OFFS_E, 4},
    {OFFS_E, 5},
    {OFFS_E, 6},
    {OFFS_E, 1},

    //25-32
    {OFFS_D, 3},
    {OFFS_D, 4},
    {OFFS_D, 5},
    {OFFS_D, 6},
    {OFFS_D, 7},
    {OFFS_G, 0},
    {OFFS_J, 0},
    {OFFS_J, 1},

    //33-40
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
const latch_info_t zif2vdd[40] = {
    //1
    {3, 7},
    {3, 4},
    {3, 5},
    {4, 0},

    //5
    {3, 2},
    {4, 2},
    {2, 6},
    {2, 1},

    //9
    {2, 2},
    {2, 3},
    {2, 0},
    {2, 7},

    //13
    {2, 4},
    {0, -1},
    {0, -1},
    {0, -1},

    //17
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},

    //21
    {2, 5},
    {0, -1},
    {0, -1},
    {0, -1},

    //25
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},

    //29
    {0, -1},
    {4, 6},
    {0, -1},
    {4, 1},

    //33
    {4, 5},
    {4, 3},
    {4, 4},
    {4, 7},

    //37
    {3, 3},
    {3, 6},
    {3, 0},
    {3, 1},
};

const latch_info_t zif2vpp[40] = {
    //1
    {0, 2},
    {0, 3},
    {1, 2},
    {1, 3},

    //5
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},

    //9
    {1, 5},
    {1, 4},
    {0, -1},
    {0, -1},

    //13
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},

    //17
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},

    //21
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},

    //25
    {0, -1},
    {0, -1},
    {0, -1},
    {0, -1},

    //29
    {0, -1},
    {0, 0},
    {1, 0},
    {0, 7},

    //33
    {1, 6},
    {1, 1},
    {0, -1},
    {0, 1},

    //37
    {1, 7},
    {0, 6},
    {0, 5},
    {0, 4},
};

const latch_info_t zif2gnd[40] = {
    {6, 2},
    {6, 3},
    {6, 6},
    {6, 1},

    {7, 2},
    {7, 3},
    {7, 6},
    {7, 1},

    {7, 0},
    {7, 7},
    {7, 4},
    {7, 5},

    {0, -1},
    {5, 3},
    {0, -1},
    {5, 6},

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
    {5, 2},
    {6, 0},
    {6, 7},

    {0, -1},
    {6, 4},
    {6, 5},
    {5, 5},

    {5, 4},
    {5, 1},
    {5, 7},
    {5, 0},
};

static unsigned char latch_mirror[8]; /* Read mirror of the current latch state. */


void dir_write(zif_bits_t zif_val)
{
    port_bits_t port_val = {0};

    DEBUG(print_zif_bits("  dir_write zif_bits", zif_val));

    //Read control signals
    dir_read_all(port_val);
    zif_pins_to_ports(zif_val, port_val);
    DEBUG(print_port_bits("  dir_write port_bits", port_val));
    dir_write_all(port_val);
}

void dir_read(zif_bits_t zif_val)
{
    port_bits_t port_val = {0};

    dir_read_all(port_val);
    DEBUG(print_port_bits("  dir_read port_bits", port_val));
    ports_to_zif_pins(port_val, zif_val);
    DEBUG(print_zif_bits("  dir_read zif_bits", zif_val));
}

void zif_write(zif_bits_t zif_val)
{
    port_bits_t port_val = {0};

    DEBUG(print_zif_bits("  zif_write zif_bits", zif_val));

    //Read control signals
    port_read_all(port_val);
    zif_pins_to_ports(zif_val, port_val);
    DEBUG(print_port_bits("  zif_write port_bits", port_val));
    port_write_all(port_val);
}

void zif_read(zif_bits_t zif_val)
{
    port_bits_t port_val = {0};

    port_read_all(port_val);
    DEBUG(print_port_bits("  zif_read port_bits", port_val));

    ports_to_zif_pins(port_val, zif_val);
    DEBUG(print_zif_bits("  zif_read zif_bits", zif_val));
}

void zif_pins_to_ports(zif_bits_t zif, port_bits_t port)
{
    for(unsigned int pin_no = 0; pin_no < (sizeof(zif2port)/sizeof(port_info_t)); pin_no++)
    {
        /* It would be nice to assume the compiler can divide by 8 using
        shifts, but XC8 is broken in free mode. */
        unsigned char set_of_8 = (pin_no >> 3);
        unsigned char bit_offset = 1 << (pin_no & 0x07);

        unsigned char pin_val = (zif[set_of_8] & bit_offset) ? 1 : 0;

        port_info_t curr = zif2port[pin_no];
        port[curr.bank] &= ~(1 << curr.offset); /* ZIF pins are shared w/
        control pins. Don't accidentally erase control pin values. */
        port[curr.bank] |= (pin_val << curr.offset);
    }
}


void ports_to_zif_pins(port_bits_t port, zif_bits_t zif)
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
void port_read_all(port_bits_t p_bits)
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

void port_write_all(port_bits_t p_bits)
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

void dir_read_all(port_bits_t p_bits)
{
    p_bits[0] = TRISA;
    p_bits[1] = TRISB;
    p_bits[2] = TRISC;
    p_bits[3] = TRISD;
    p_bits[4] = TRISE;
    p_bits[5] = TRISF;
    p_bits[6] = TRISG;
    p_bits[7] = TRISH;
    p_bits[8] = TRISJ;
}

void dir_write_all(port_bits_t p_bits)
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
/*
void zifdir_mask(unsigned char (*)[5])
{

}
*/


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

    latch_cache[latch_no] = val;
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

int vpp_state(void)
{
    return OE_VPP;
}

void vdd_en(void)
{
    OE_VDD = 0;
}

void vdd_dis(void)
{
    OE_VDD = 1;
}

int vdd_state(void)
{
    return OE_VDD;
}


int OEn_state(void) {
    return (LE7 << 7) | (LE6 << 6) | (LE5 << 5) | (LE4 << 4) | (LE3 << 3) | (LE2 << 2) | (LE1 << 1) | (LE0 << 0);
}

void set_vpp(const_zif_bits_t zif)
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

void set_vdd(const_zif_bits_t zif)
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

    write_latch(2, ~latch_vdd_masks[0]);
    write_latch(3, ~latch_vdd_masks[1]);
    write_latch(4, ~latch_vdd_masks[2]);
}

void set_gnd(const_zif_bits_t zif)
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

void pupd(int tristate, int val) {
    PUPD_TRIS = tristate;
    PUPD_PORT = val;
}

void print_port_bits(const char *prefix, port_bits_t p_bits) {
    printf("%s: A:%02X B:%02X C:%02X D:%02X E:%02X F:%02X G:%02X H:%02X J:%02X\r\n",
            prefix,
            p_bits[0], p_bits[1], p_bits[2], p_bits[3], p_bits[4],
            p_bits[5], p_bits[6], p_bits[7], p_bits[8]);
}

void print_zif_bits(const char *prefix, zif_bits_t zif_val) {
    printf("%s: %02X %02X %02X %02X %02X\r\n",
            prefix,
            zif_val[0], zif_val[1], zif_val[2], zif_val[3], zif_val[4]);
}

void print_latch_bits(const char *prefix, latch_bits_t lb) {
    printf("%s: 0:%02X 1:%02X 2:%02X 3:%02X 4:%02X 5:%02X 6:%02X 7:%02X\r\n",
            prefix,
            lb[0], lb[1], lb[2], lb[3],
            lb[4], lb[5], lb[6], lb[7]);
}

