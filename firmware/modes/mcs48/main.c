#include <xc.h>

#include "../../arglib.h"
#include "../../comlib.h"
#include "../../io.h"
#include "../../mode.h"
#include "../../stock_compat.h"
#include "../../system.h"


/*
 * Pin map:
 *   DUT   DUT  DUT  ZIF Prog
 *   Func  Lbl  Pin  Pin Func
 *
 *   D/A0  DB0  12   17  RE0
 *   D/A1  DB1  13   24  RE1
 *   D/A2  DB2  14   19  RE2
 *   D/A3  DB3  15   20  RE3
 *   D/A4  DB4  16   21  RE4
 *   D/A5  DB5  17   22  RE5
 *   D/A6  DB6  18   23  RE6
 *   D/A7  DB7  19   18  RE7
 *     A8  P20  21   13  RD0
 *     A9  P21  22   14  RD1
 *    A10  P22  23   15  RD2
 *
 *    +5V  Vcc  40   40  VDD
 *    +5V  Vdd  26   39  VDD
 *        PROG  25   38
 *   +12V   EA   7   37  VPP
 *
 *      /RESET   4    5  RJ7
 *          T0   1    6  RJ6
 *       XTAL1   2    4  P1A/RC2
 *         ALE  11    3  RC3
 */


#define PIN_RESET PORTJbits.RJ7


// Vcc = P40
// Vdd = P39
zif_bits_t pins_vdd = {
    0x00, 0x00, 0x00, 0x00, 0xC0
};

// EA = P37
zif_bits_t pins_vpp = {
    0x00, 0x00, 0x00, 0x00, 0x10
};

// Vss = P01
zif_bits_t pins_gnd = {
    0x01, 0x00, 0x00, 0x00, 0x00
};

static inline void delay_tmr2(int cycles)
{
    for (; cycles > 0; cycles--) {
        PIR1bits.TMR2IF = 0;
        while (!PIR1bits.TMR2IF) NOP();
    }
}


static void dev_init()
{
    io_init();
    LED = 1;

    vdd_val(VDD_51);  //  5V
    vpp_val(VPP_126); // 12V

    set_gnd(pins_gnd);
    set_vdd(pins_vdd);
    set_vpp(pins_vpp);

    TRISE = 0x00; // E0-E7 output
    TRISD = 0xF8; // D0-D2 output, D3-D7 input
    TRISJ = 0x3F; // J6-J7 output, J0-J5 input
    TRISC = TRISC & ~0x04; // C2 output

    vdd_en();

    // generate 1.5 MHz clock on P4 for XTAL1
    T2CON = 0x04;   // enable T2, prescale 1:1
    PR2 = 7;        // 1.5 MHz
    CCPR1L = 4;     // 50% duty cycle
    CCP1CON = 0x0C; // enable ECCP1 as PWM, active-high

    vpp_en();
}

static void dev_off()
{
    io_init();
    LED = 0;
}

static uint8_t read_byte(uint16_t addr)
{
    uint8_t value;

    PORTE = addr & 0xFF;
    PORTD = (addr >> 8) & 0x07;
    delay_tmr2(16);

    PIN_RESET = 1;
    TRISE = 0xFF;
    delay_tmr2(16);

    value = PORTE;

    PIN_RESET = 0;
    TRISE = 0x00;

    return value;
}

static void print_read(uint16_t addr, uint16_t length)
{
    dev_init();
    printf("%04X ", addr);
    for (uint16_t idx = 0; idx < length; idx++) {
        printf("%02X ", read_byte(addr + idx));
    }
    printf("\r\n");
    dev_off();
}


static inline void eval_command(char *cmd)
{
    char *cmd_tok = strtok(cmd, " ");
    switch (cmd_tok[0]) {
    case 'r':
        ;
        uint16_t addr   = xtoi(strtok(NULL, " "));
        uint16_t length = xtoi(strtok(NULL, " "));
        print_read(addr, length);
        break;


    case 'b':
        stock_reset_to_bootloader();
        break;

    default:
        printf("ERROR: unknown command '%s'\r\n", cmd_tok);
        break;
    }
}

void mode_main(void)
{
    LED = 0;

    while (1) {
        eval_command(com_cmd_prompt());
    }
}

void interrupt high_priority isr()
{
    usb_service();
}
