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
    return 0;
}
