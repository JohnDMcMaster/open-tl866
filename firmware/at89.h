#ifndef AT89_H
#define AT89_H

unsigned char at89_read(unsigned int addr);
void at89_write(unsigned int addr, unsigned char data);
void at89_erase();
void at89_lock(unsigned char mode);
unsigned char at89_read_sysflash(unsigned int offset);
unsigned char at89_read_sig(unsigned int offset);

#endif

