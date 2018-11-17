#include <xc.h>

#include "io.h"
#include "system.h"
#include "comlib.h"
#include "at89.h"

/*
CMD> r 0 10
Could not detect an AT89C51. Ignoring command.
Please make sure the target is inserted in the correct orientation.
CMD> r 0 10
000 00 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF

4/4 torture fail

XXX: is this TL866 switching time or at89c51 power up delay?
*/
#define VDD_DELAY   10
#define VPP_DELAY   10

#define ZIFMASK_XTAL1 4;
#define ZIFMASK_GND 8;
#define ZIFMASK_VDD 128;
#define ZIFMASK_VPP 64;
#define ZIFMASK_PROG 32;

const_zif_bits_t at89_gnd        = {0, 0, 0x8, 0, 0};
const_zif_bits_t at89_vdd        = {0, 0, 0, 0, 0x80};
const_zif_bits_t at89_vpp        = {0, 0, 0, 0x40, 0};

//Data bus tristated (data in)
const_zif_bits_t at89_dir_din = {  0,
                    0b00100000,   //Busy signal (14)
                    0,
                    0b10000000,   //p0.7 (32)
                    0b01111111 }; //p0.{6-0} (33-39)
//Data out
const_zif_bits_t at89_dir_dout = {  0,
                    0b00100000,   //Busy signal (14)
                    0,
                    0,
                    0 };

//Neat trick taken from a stack overflow answer.
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

static inline void clear_prog(zif_bits_t op_base)
{
    op_base[3] &= 0xFF ^ 0x20;
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
    //Filter the zif_bits response into a char byte with P0 bits
                   //Trim non-data ZIF pins    //Set the LSB of data byte
    unsigned char byte = (zif_state[4] << 1) | !! (zif_state[3] & (1 << 7));

    //Invert bit-endianness
    return invert_bit_endianness(byte);
}

static inline void clock_write(zif_bits_t op, unsigned int cycles)
{
    zif_write(op);
    for(unsigned int i = 0; i <= cycles; i++) {
        at89_pin_flip_clock();
        __delay_us(1);
        at89_pin_flip_clock();
        __delay_us(1);
    }
}

/*
//Very slow, but useful for prototyping when other
//pins need to be changed alongside the clock
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

void at89_mode(bool p26, bool p27, bool p36, bool p37, zif_bits_t io_out) {
    if (p26) {
        io_out[3] |= 0b00000100;
    } else {
        io_out[3] &= 0xFF ^ 0b00000100;
    }

    if (p27) {
        io_out[3] |= 0b00001000;
    } else {
        io_out[3] &= 0xFF ^ 0b00001000;
    }

    if (p36) {
        io_out[1] |= 0b10000000;
    } else {
        io_out[1] &= 0xFF ^ 0b10000000;
    }

    if (p37) {
        io_out[2] |= 0b00000001;
    } else {
        io_out[2] &= 0xFF ^ 0b00000001;
    }

    zif_write(io_out);
}

void at89_idle(zif_bits_t io_out) {
    //Read
    at89_mode(0, 0, 1, 1, io_out);
}

void at89_on(bool vpp, bool dout, zif_bits_t io_out) {
    /*
    Table Flash Programming Modes

    vpp: set VPP pins and target, but don't enable
    dout: drive data out
    p26: port 2, pin 6 value
    p27: port 2, pin 7 value
    p36: port 3, pin 6 value
    p37: port 3, pin 7 value
    */

    io_init_z();

    if (dout) {
        dir_write(at89_dir_dout);
    } else {
        dir_write(at89_dir_din);
    }

    set_gnd(at89_gnd);

    //Set voltages
    vdd_val(VDD_51); //5.0 v - 5.2 v
    //Set Vdd / GND pinout
    set_vdd(at89_vdd);

    if (vpp) {
        set_vpp(at89_vpp);
        vpp_val(VPP_126); //12.8 v - 13.2 v
    }

    //Base pin setting
    io_out[0] = 0b00000000;
    io_out[1] = 0b00000001; //RST (9)
    io_out[2] = 0b00000000;
    //Prog always starts high
    io_out[3] = 0b01100000;   //VPP (31), PROG (30)
    io_out[4] = 0b00000000;

    //Default to read state?
    at89_mode(0, 0, 1, 1, io_out);

    vdd_en();
    __delay_ms(VDD_DELAY);
}

unsigned char at89_read(unsigned int addr)
{
    zif_bits_t io_out;

    at89_on(false, false, io_out);
    at89_mode(0, 0, 1, 1, io_out);

    //Allocate an empty zifbits struct for reading pin state
    zif_bits_t response    = { 0, 0, 0, 0, 0 };

    //Mask in the address bits to the appropriate pins
    mask_addr(io_out, addr);

    //Wait tAVQV
    clock_write(io_out, 48);

    //Read the current pin state (to read in the requested byte)
    zif_read(response);

    at89_idle(io_out);
    at89_off();

    return zif_to_data(response);
}

bool at89_wait_idle(void) {
    //Wait until BSYn high to indicate done
    //tWC => 2.0 ms
    //16 MHz osc => should complete within 32k cycles (let alone loops)
    for (int i = 0; i < 32000; ++i) {
        zif_bits_t response;
        zif_read(response);
        //P3.4 => 14
        if (response[1] & 0x20) {
            return true;
        }
    }
    return false;
}

void at89_write(unsigned int addr, unsigned char data)
{
    /*
    PROGn should be pulsed low between 1 - 110 us
    */

    printf("Writing %02X at %03X... ", data, addr);

    zif_bits_t io_out;
    at89_on(true, true, io_out);
    at89_mode(0, 1, 1, 1, io_out);

    //Wait tEHSH
    clock_write(io_out, 48);

    //Enable VPP right before setting the ZIF state
    vpp_en();

    //Mask in our address and data outputs to the base pin configuration
    mask_addr(io_out, addr);
    mask_data(io_out, data);
    zif_write(io_out);

    /*
    Wait before lowering ALE/PROGn
    tSHGL: 10 us

    tAVGL: 48 clocks
    tDVGL: 48 clocks
    Noting address, data were set at the same time
    */
    clock_write(io_out, 48);
    //How long does above take? Maybe we could just skip this
    __delay_us(10);

    //Bring PROGn low to begin program
    clear_prog(io_out);
    zif_write(io_out);

    //Wait tGLGH (1-110 us)
    __delay_us(65);

    //Bring PROGn high to stop program
    mask_prog(io_out);
    //Wait tGHBL, tGHDX, tGHAX
    clock_write(io_out, 48);

    if (!at89_wait_idle()) {
        printf("ERROR: BUSYn timeout, programming failed\r\n");
    }

    //Wait tGHSL before bringing VPP low
    //Should have plenty of delay from waiting idle
    at89_idle(io_out);
    at89_off();

    //The client / user is expected to verify this with a read command.
    printf("done.\r\n");
}

void at89_erase()
{
    printf("Erasing... ");

    zif_bits_t io_out;
    at89_on(true, true, io_out);
    at89_mode(1, 0, 0, 0, io_out);

    //Wait tEHSH? erase timing is sort of vague
    //lets do it just in case
    clock_write(io_out, 48);

    //Enable VPP right before setting the ZIF state
    vpp_en();

    //Wait tSHGL (10 us)
    __delay_us(10);

    //FIXME
    __delay_ms(20);

    //Start erase by brining PROGn low
    clear_prog(io_out);
    zif_write(io_out);

    //Erase function requires 10ms prog pulse
    __delay_ms(10);

    //Raise PROGn to complete erase
    mask_prog(io_out);
    zif_write(io_out);

    //Wait tGHSL before bringing VPP low
    __delay_us(10);

    at89_idle(io_out);
    at89_off();

    printf("done.\r\n");
}

void at89_lock(unsigned char mode)
{
    /*
     * AT89C51 lock (modes 2, 3, 4) Pinout:
     *
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                 //(high)
     * PSEN     <-      29          RD7                 //(low)
     * ALE      <-      30          RG0                 //Pulsed write lock
     * VPP      <-      31          VPP_31              //12v
     * VCC      <-      40          Vdd_40
     * P2.6     <-      27          RD5                 //ctrl (high)
     * P2.7     <-      28          RD6                 //ctrl (2:h, 3:h, 4:l)
     * P3.4     ->      14          RD1                 //Busy
     * P3.6     <-      16          RG1                 //ctrl (2:h, 3:l, 4:h)
     * P3.7     <-      17          RE0                 //ctrl (2:h, 3:l, 4:l)
     */

    printf("Locking with mode %u... ", mode);

    //Base pin setting for erasing
    zif_bits_t lock_1 = { 0b00000000,
                             0b00000001, //RST (9)
                             0b00000000,
                             0b01000000, //VPP (31)
                             0b00000000 };

    zif_bits_t lock_2 = { 0b00000000,
                             0b00000001, //RST (9)
                             0b00000000,
                             0b01000100, //VPP (31), 2.6 (27)
                             0b00000000 };

    zif_bits_t lock_3 = { 0b00000000,
                             0b00000001, //RST (9)
                             0b00000000,
                             0b01000100, //VPP (31), 2.6 (27)
                             0b00000000 };

    zif_bits_t dir = {  0,
                        0b00100000,   //Busy signal (14)
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

    //Making a pulsed lock_2
    zif_bits_t lock_2_proglow;
    memcpy(lock_2_proglow, lock_2, 5);
    mask_prog(lock_2);

    //Making a pulsed lock_3
    zif_bits_t lock_3_proglow;
    memcpy(lock_3_proglow, lock_3, 5);
    mask_prog(lock_3);

    //Set pin direction
    dir_write(dir);

    //Set pins
    set_vdd(at89_vdd);
    set_vpp(at89_vpp);
    set_gnd(at89_gnd);

    //Set voltages
    vdd_val(VDD_51); //5.0 v - 5.2 v
    vpp_val(VPP_126); //12.8 - 13.2
    vdd_en();
    __delay_ms(VDD_DELAY);

    //Enable VPP right before setting the ZIF state
    vpp_en();
    __delay_ms(VPP_DELAY);

    //Using clock_write(...) results in some inconsistency, and being unable
    //to set pins while the clock is running makes it rather unflexible.
    //Working around this limitation by making multiple calls to clock_write
    //results in gaps on the clock line while writing to sreg with the new pin
    //state. This happens to hinder the following algorithm, so the clock here
    //will be driven by the PIC18F87J50 PWM module as described in sections
    //17.4, 18.1 and 18.4 of the datasheet. Eventually this should be a
    //drop-in replacement function for clock_write(...)

    //Enable PWM clock.
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

    T2CON = 0;     //Enable TMR2 with prescaler = 1
    CCP2CON = 0;   //Disable PWM on CCP1

    ///////////////////////////////////////////////////////////////////////////

    at89_off();

    //The client / user is expected to verify this with a read command
    //or a blank check command. A slight timing invariance could also be used
    //to discern blank and protected chips. (TODO)
    printf("done.\r\n");
}

unsigned char at89_read_sysflash(unsigned int offset)
{
    zif_bits_t io_out;

    at89_on(false, false, io_out);
    at89_mode(0, 0, 0, 0, io_out);

    //Allocate an empty zifbits struct for reading pin state
    zif_bits_t response  = { 0, 0, 0, 0, 0 };

    //Mask in the address bits to the appropriate pins
    mask_addr(io_out, offset);

    //Give the clock on/off states to zif_clock_write(..) and loop 48 cycles
    clock_write(io_out, 48);

    //Read the current pin state (to read in the requested byte)
    zif_read(response);

    at89_idle(io_out);
    at89_off();

    return zif_to_data(response);
}

unsigned char at89_read_sig(unsigned int offset)
{
    return at89_read_sysflash(0x30 + offset);
}

void at89_off(void) {
    io_init_0();
    /*
    When doing io_init_z
    60 ms fail
    70 ms ok
    Add 50% margin => 105
    */
    __delay_ms(105);
}

