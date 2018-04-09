#include <xc.h>


// Default bootloader needs this constant, otherwise application won't be
// recognized.
const unsigned char sigbytes[] @ 0x1FBFC = {0x55, 0xAA, 0xA5, 0x5A} ;

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
