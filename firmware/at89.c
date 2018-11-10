#include <xc.h>

#include "io.h"
#include "system.h"
#include "comlib.h"


#define ZIFMASK_XTAL1 4;
#define ZIFMASK_GND 8;
#define ZIFMASK_VDD 128;
#define ZIFMASK_VPP 64;
#define ZIFMASK_PROG 32;

static zif_bits_t zbits_null = {0, 0, 0, 0, 0};
static zif_bits_t gnd        = {0, 0, 0x8, 0, 0};
static zif_bits_t vdd        = {0, 0, 0, 0, 0x80};
static zif_bits_t vpp        = {0, 0, 0, 0x40, 0};

// Neat trick taken from a stack overflow answer.
static inline unsigned char invert_bit_endianness(unsigned char byte)
{
    static unsigned char lookup[16] = {
                            0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
                            0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };
    return (lookup[byte & 0b1111] << 4) | lookup[byte >> 4];
}

static inline void mask_p2_7(zif_bits_t op_base)
{
    op_base[3] |= 0x8;
}

static inline void mask_p3_6(zif_bits_t op_base)
{
    op_base[1] |= 0x80;
}

static inline void mask_p3_7(zif_bits_t op_base)
{
    op_base[2] |= 0x1;
}

/*
static inline void mask_xtal1(zif_bits_t op_base)
{
    op_base[2] |= 0x4;
}
*/

static inline void mask_prog(zif_bits_t op_base)
{
    op_base[3] |= 0x20;
}

static inline void mask_addr(zif_bits_t op_base, unsigned int addr)
{
    op_base[0] = addr & 0xFF;
    op_base[2] |= (addr >> 8) << 4;
}

static inline void mask_data(zif_bits_t op_base, unsigned char data)
{
    op_base[3] |= (data & 0x80);
    op_base[4] |= (invert_bit_endianness(data & 0x7f) >> 1);
}

static inline unsigned char zif_to_data(zif_bits_t zif_state)
{
    // Filter the zif_bits response into a char byte with P0 bits
                   // Trim non-data ZIF pins    // Set the LSB of data byte
    unsigned char byte = (zif_state[4] << 1) | !! (zif_state[3] & (1 << 7));

    // Invert bit-endianness
    return invert_bit_endianness(byte);
}

// Flip clock pin directly from TL866
static inline void pin_flip_clock()
{
    PORTE ^= 0x4;
}

static inline void clock_write(zif_bits_t op, unsigned int cycles)
{
    zif_write(op);
    for(unsigned int i = 0; i <= cycles; i++) {
        pin_flip_clock();
        __delay_us(1);
        pin_flip_clock();
        __delay_us(1);
    }
}

/*
// Very slow, but useful for prototyping when other
// pins need to be changed alongside the clock
static inline void zif_clock_write(zif_bits_t op_template, zif_bits_t op_clk,
                            unsigned int cycles
                            )
{
    for(unsigned char i = 0; i <= cycles; i++) {
        zif_write(op_template);
        zif_write(op_clk);
    }
}
*/

unsigned char at89_read(unsigned int addr)
{
    /* 
     * AT89C51 Read Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                     // (high)
     * PSEN     <-      29          RD7                     // (low)
     * PROG     <-      30          RG0                     // (high)
     * VPP      <-      31          RJ0                     // (high)
     * VCC      <-      40          Vdd_40
     * P0.{0-7) ->      39-32       RB{6,5,4,3,2}, RJ{3,2,1}      // PGM Data
     * P1.{0-7} <-      1-8         RC{5,4,3,2}, RJ{7,6}, RC{6,7} // Addr
     * P2.{0-3} <-      21-24       RE{4,5,6,1}             // Addr (contd.)
     * P2.6     <-      27          RD5                     // ctrl (low)
     * P2.7     <-      28          RD6                     // ctrl (low)
     * P3.4     ->      14          RD1                     // Busy
     * P3.6     <-      16          RG1                     // ctrl (high)
     * P3.7     <-      17          RE0                     // ctrl (high)
     */
    
    // Set pin direction
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0,
                        0b10000000,   // p0.7 (32)
                        0b01111111 }; // p0.{6-0} (33-39)  
    dir_write(dir);
    
    // Set Vdd / GND pinout  
    set_vdd(vdd);
    set_gnd(gnd);
    
    // Set voltages
    vdd_val(5); // 5.0 v - 5.2 v

    // Allocate an empty zifbits struct for reading pin state
    zif_bits_t response    = { 0, 0, 0, 0, 0 };

    // Base pin setting for reading
    zif_bits_t read_base = { 0b00000000,
                             0b10000001,   // 3.6 ctrl (16), RST (9)
                             0b00000001,   // 3.7 ctrl (17) 
                             0b01100000,   // VPP (31), PROG (30)
                             0b00000000 };

    zif_bits_t read_clk;

    // Mask in the address bits to the appropriate pins
    mask_addr(read_base, addr);

    // Give the clock on/off states to zif_clock_write(..) and loop 48 cycles
    clock_write(read_base, 48);

    // Read the current pin state (to read in the requested byte)
    zif_read(response);

    // We're done with the byte. Turn off all outputs.
    zif_write(zbits_null);

    return zif_to_data(response);
}

void at89_write(unsigned int addr, unsigned char data)
{
    /* 
     * AT89C51 write Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                     // (high)
     * PSEN     <-      29          RD7                     // (low)
     * PRO      <-      30          RG0                     // Pulsed prog.
     * VPP      <-      31          VPP_31                  // 12v
     * VCC      <-      40          Vdd_40
     * P0.{0-7) <-      39-32       RB{6,5,4,3,2}, RJ{3,2,1}      // PGM Data
     * P1.{0-7} <-      1-8         RC{5,4,3,2}, RJ{7,6}, RC{6,7} // Addr
     * P2.{0-3} <-      21-24       RE{4,5,6,1}             // Addr (contd.)
     * P2.6     <-      27          RD5                     // ctrl (low)
     * P2.7     <-      28          RD6                     // ctrl (high)
     * P3.4     ->      14          RD1                     // Busy
     * P3.6     <-      16          RG1                     // ctrl (high)
     * P3.7     <-      17          RE0                     // ctrl (high)
     */
    
    printf("Writing %02X at %03X... ", data, addr);

    // Set pin direction
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0, 0, 0 };
    dir_write(dir);
    
    // Set pins
    set_vdd(vdd);
    set_vpp(vpp);
    set_gnd(gnd);
    
    // Set voltages
    vdd_val(5); // 5.0 v - 5.2 v
    vpp_val(1); // 12.8 v - 13.2 v

    // Base pin setting for writing
    zif_bits_t write_base = { 0b00000000,
                              0b10000001,   // 3.6 ctrl (16), RST (9)
                              0b00000001,   // 3.7 ctrl (17) 
                              0b01001000,   // VPP (31), 2.7 (28)
                              0b00000000 };
    
    // Mask in our address and data outputs to the base pin configuration
    mask_addr(write_base, addr);
    mask_data(write_base, data);
    
    // Create a zif state to set before running the clock
    // PROG needs to be pulsed, can't be kept low the entire time.
    zif_bits_t write_preclk;
    memcpy(write_preclk, write_base, 5);
    mask_prog(write_preclk);
   
    // Enable VPP right before setting the ZIF state
    vpp_en();

    // Set PROG high before pulsing it low during programming
    zif_write(write_preclk);
    __delay_us(20); // 20us might be too generous. TODO

    clock_write(write_base, 48);

    // We're done. Disable VPP and reset the ZIF state.
    vpp_dis();
    zif_write(zbits_null);
    
    // The client / user is expected to verify this with a read command.
    printf("done.\r\n");
}

void at89_erase()
{
    /* 
     * AT89C51 erase Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                     // (high)
     * PSEN     <-      29          RD7                     // (low)
     * ALE      <-      30          RG0                     // Pulsed erase
     * VPP      <-      31          VPP_31                  // 12v
     * VCC      <-      40          Vdd_40
     * P2.6     <-      27          RD5                     // ctrl (high)
     * P2.7     <-      28          RD6                     // ctrl (low)
     * P3.4     ->      14          RD1                     // Busy
     * P3.6     <-      16          RG1                     // ctrl (low)
     * P3.7     <-      17          RE0                     // ctrl (low)
     */
    
    printf("Erasing... ");
    
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0, 0, 0 };
    
    // Set pin direction
    dir_write(dir);
    
    // Set pins
    set_vdd(vdd);
    set_vpp(vpp);
    set_gnd(gnd);
    
    // Set voltages
    vdd_val(5); // 5.0 v - 5.2 v
    vpp_val(1); // 12.8 - 13.2
    
    // Base pin setting for erasing
    zif_bits_t erase_base =     {       0b00000000,
                                        0b00000001, // RST (9)
                                        0b00000000,  
                                        0b01000100, // VPP (31), 2.6 (27)
                                        0b00000000 };

    // Create a zif state to set before running the clock
    // PROG needs to be pulsed, can't be kept low the entire time.
    zif_bits_t erase_preclk;
    memcpy(erase_preclk, erase_base, 5);
    mask_prog(erase_preclk);
    
    // Enable VPP right before setting the ZIF state
    vpp_en();
    
    // Set PROG high before pulsing it low during erase
    zif_write(erase_preclk);
    __delay_ms(20);
    
    clock_write(erase_base, 48);
    
    // Erase function requires 10ms prog pulse
    __delay_ms(10);
    
    // We're done. Disable VPP and reset the ZIF state.
    vpp_dis();
    zif_write(zbits_null);
    
    // The client / user is expected to verify this with a read command
    // or a blank check command (TODO)
    printf("done.\r\n");
}

void at89_lock(unsigned char mode)
{
    /* 
     * AT89C51 lock (modes 2, 3, 4) Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                 // (high)
     * PSEN     <-      29          RD7                 // (low)
     * ALE      <-      30          RG0                 // Pulsed write lock
     * VPP      <-      31          VPP_31              // 12v
     * VCC      <-      40          Vdd_40
     * P2.6     <-      27          RD5                 // ctrl (high)
     * P2.7     <-      28          RD6                 // ctrl (2:h, 3:h, 4:l)
     * P3.4     ->      14          RD1                 // Busy
     * P3.6     <-      16          RG1                 // ctrl (2:h, 3:l, 4:h)
     * P3.7     <-      17          RE0                 // ctrl (2:h, 3:l, 4:l)
     */
    
    printf("Locking with mode %u... ", mode);
    
    // Base pin setting for erasing
    zif_bits_t lock_1 = { 0b00000000,
                             0b00000001, // RST (9)
                             0b00000000,  
                             0b01000000, // VPP (31)
                             0b00000000 };

    zif_bits_t lock_2 = { 0b00000000,
                             0b00000001, // RST (9)
                             0b00000000,  
                             0b01000100, // VPP (31), 2.6 (27)
                             0b00000000 };

    zif_bits_t lock_3 = { 0b00000000,
                             0b00000001, // RST (9)
                             0b00000000,  
                             0b01000100, // VPP (31), 2.6 (27)
                             0b00000000 };
    
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0, 0, 0 };


    mask_prog(lock_1);

    mask_p2_7(lock_2);
    mask_p3_6(lock_2);
    mask_p3_7(lock_2);

    switch (mode) {
        case 2:
            printf("2\r\n");
            mask_p2_7(lock_3);
            mask_p3_6(lock_3);
            mask_p3_7(lock_3);
            break;
        case 3:
            printf("3\r\n");
            mask_p2_7(lock_3);
            break;
        case 4:
            printf("4\r\n");
            mask_p3_6(lock_3);
            break;
        default:
            printf("Invalid mode %u. Valid modes are 2, 3 or 4\r\n", mode);
            return;
    }

    // Making a pulsed lock_2
    zif_bits_t lock_2_proglow;
    memcpy(lock_2_proglow, lock_2, 5);
    mask_prog(lock_2);

    // Making a pulsed lock_3
    zif_bits_t lock_3_proglow;
    memcpy(lock_3_proglow, lock_3, 5);
    mask_prog(lock_3);

    // Set pin direction
    dir_write(dir);
    
    // Set pins
    set_vdd(vdd);
    set_vpp(vpp);
    set_gnd(gnd);
    
    // Set voltages
    vdd_val(5); // 5.0 v - 5.2 v
    vpp_val(1); // 12.8 - 13.2

    
    // Enable VPP right before setting the ZIF state
    vpp_en();
   
    // Using clock_write(...) results in some inconsistency, and being unable
    // to set pins while the clock is running makes it rather unflexible.
    // Working around this limitation by making multiple calls to clock_write
    // results in gaps on the clock line while writing to sreg with the new pin
    // state. This happens to hinder the following algorithm, so the clock here
    // will be driven by the PIC18F87J50 PWM module as described in sections
    // 17.4, 18.1 and 18.4 of the datasheet. Eventually this should be a
    // drop-in replacement function for clock_write(...)

    // Enable PWM clock. 
    CCP2CON = 0b10001100;
    TRISEbits.RE2 = 0;
    T2CON = 0b00000100;
    PR2 = 249;
    CCPR1L = 125;

    zif_write(lock_1);
    __delay_ms(400);

    zif_write(lock_2);
    __delay_ms(1);

    zif_write(lock_2_proglow);
    __delay_ms(1);

    zif_write(lock_2);
    __delay_ms(400);

    zif_write(lock_3);
    __delay_ms(1);

    zif_write(lock_3_proglow);
    __delay_ms(1);

    zif_write(lock_3);
    __delay_ms(400);
    
    T2CON = 0;     // Enable TMR2 with prescaler = 1
    CCP2CON = 0;   // Disable PWM on CCP1

    ///////////////////////////////////////////////////////////////////////////

    // We're done. Disable VPP and reset the ZIF state.
    vpp_dis();
    zif_write(zbits_null);
    
    // The client / user is expected to verify this with a read command
    // or a blank check command. A slight timing invariance could also be used
    // to discern blank and protected chips. (TODO)
    printf("done.\r\n");
}

unsigned char at89_read_sysflash(unsigned int offset)
{
   // TODO. Implements the signature reading routine as described in the
   // datasheet. Would be a good precheck before doign read/write/erase ops.
    
     /* 
     * AT89C51 Read Signature Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                     // (high)
     * PSEN     <-      29          RD7                     // (low)
     * PROG     <-      30          RG0                     // (high)
     * VPP      <-      31          RJ0                     // (high)
     * VCC      <-      40          Vdd_40
     * P0.{0-7) ->      39-32       RB{6,5,4,3,2}, RJ{3,2,1}      // PGM Data
     * P1.{0-7} <-      1-8         RC{5,4,3,2}, RJ{7,6}, RC{6,7} // Addr
     * P2.{0-3} <-      21-24       RE{4,5,6,1}             // Addr (contd.)
     * P2.6     <-      27          RD5                     // ctrl (low)
     * P2.7     <-      28          RD6                     // ctrl (low)
     * P3.4     ->      14          RD1                     // Busy
     * P3.6     <-      16          RG1                     // ctrl (low)
     * P3.7     <-      17          RE0                     // ctrl (low)
     */
    
    // Set pin direction
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0,
                        0b10000000,   // p0.7 (32)
                        0b01111111 }; // p0.{6-0} (33-39)  
    dir_write(dir);
    
    // Set Vdd / GND pinout  
    set_vdd(vdd);
    set_gnd(gnd);
    
    // Set voltages
    vdd_val(5); // 5.0 v - 5.2 v
    
    // Allocate an empty zifbits struct for reading pin state
    zif_bits_t response  = { 0, 0, 0, 0, 0 };

    // Base pin setting for reading
    zif_bits_t signature_base = { 0b00000000,
                             0b00000001,   // RST (9)
                             0b00000000,
                             0b01100000,   // VPP (31), PROG (30)
                             0b00000000 };

    zif_bits_t signature_clk;
    
    // Mask in the address bits to the appropriate pins
    mask_addr(signature_base, offset);

    // Give the clock on/off states to zif_clock_write(..) and loop 48 cycles
    clock_write(signature_base, 48);

    // Read the current pin state (to read in the requested byte)
    zif_read(response);

    // We're done with the byte. Turn off all outputs.
    zif_write(zbits_null);
    
    return zif_to_data(response);
}

unsigned char at89_read_sig(unsigned int offset)
{
    return at89_read_sysflash(0x30 + offset);
}

