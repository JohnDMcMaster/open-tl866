#include <xc.h>

#include "usb.h"
#include <xc.h>
#include <string.h>
#include "usb_config.h"
#include "usb_ch9.h"
#include "usb_cdc.h"


void interrupt inthi(void)
{
    
}

void interrupt low_priority intlo(void)
{
    
}


int main(void)
{    
    PORTC = 0x00;
    TRISCbits.RC0 = 0;
    PORTCbits.RC0 = 0;
    
    int blink = 0;
    
    while(1)
    {
        for(long i = 0; i < 100000; i++)
        {
        }
        
        blink ^= 0x01;
        PORTCbits.RC0 = blink;
    }
    
    return 0;
}
