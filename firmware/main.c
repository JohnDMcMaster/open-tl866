/*
 * open-tl866 firmware
 * TODO: Add description
 */

#include "usb.h"
#include "usb/usb_callbacks.c"

#include <xc.h>
#include <string.h>

#include "modes/bitbang/bitbang.h"
//#include "modes/glitch/glitch.h"

static inline void init(void) {
        unsigned int pll_startup = 600;
    OSCTUNEbits.PLLEN = 1;
    while (pll_startup--);

    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;

    WDTCONbits.ADSHR = 1;
    ANCON0 |= 0x9F; // Disable analog functionality on Ports A, F, and H.
    ANCON1 |= 0xFC;
    WDTCONbits.ADSHR = 0;

    PORTA = 0x00;
    TRISA = 0x00; // RA5-RA0: LE4, OE_VDD, LE5, LE2, LE7, LE3

    PORTB = 0x00;
    TRISB = 0x01; // RB1: Controls resistors on P15-P24. P16/P21 act especially weird.
                  // RB0: Input that detects that Vpp/Vdd voltage is okay.

    PORTC = 0x00;
    TRISC = 0x00; // RC1-RC0: ZIF Pin 20 GND driver enable, LED

    PORTD = 0x00; // All attached to ZIF
    TRISD = 0x00;

    PORTE = 0x00; // All attached to ZIF
    TRISE = 0x00;

    PORTF = 0x00;
    TRISF = 0x00; // RF7-RF5: VID_02-00, RF2: VID_12

    PORTG = 0x00;
    TRISG = 0x00; // RG4: OE_VPP,

    PORTH = 0x00;
    TRISH = 0x00; // RH7-6: VID_11-10
                  // RH5: MCU power rail shift?
                  // RH4: LE6
                  // RH3: SR_DAT
                  // RH2: SR_CLK
                  // RH1: LE1
                  // RH0: LE0

    PORTJ = 0x00; // All attached to ZIF
    TRISJ = 0x00;

    // Disable all pin drivers for initial "known" state.
    OE_VPP = 1;
    OE_VDD = 1;

    for(int i = 0; i < 2; i++)
    {
        write_latch(i, 0x00);
    }

    // PNPs- Logic 1 is off state.
    for(int i = 2; i < 5; i++)
    {
        write_latch(i, 0xff);
    }

    for(int i = 5; i < 8; i++)
    {
        write_latch(i, 0x00);
    }

    OE_VPP = 0;
    OE_VDD = 0;

    usb_init();

    PORTCbits.RC0 = 1;
}

int main(void)
{
    init();
    
    // MODE SELECT. Only one can be uncommented. TODO: Implement a config.h
    bitbang(); // Bitbang mode
    //glitch(); // future glitch mode
    // MODE SELECT END.
    
    return 0;
}

void interrupt high_priority isr()
{
    usb_service();
}
