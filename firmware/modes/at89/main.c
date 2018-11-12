#include <xc.h>

#include "../../arglib.h"
#include "../../at89.h"
#include "../../comlib.h"
#include "../../stock_compat.h"
#include "../../system.h"

int checking_sig = 1;

static inline void print_help(void)
{
    com_println("open-tl866 (at89)");
    com_println("r addr range   Read from target");
    com_println("w addr data    Write to target");
    com_println("R addr         Read sysflash from target");
    com_println("e              Erase target");
    com_println("l mode         Set lock bits to MODE (2, 3, 4)");
    com_println("s              Print signature bytes");
    com_println("S en           Enable signature check");
    com_println("B              Blank check");
    com_println("T              Run some tests");
    com_println("h              Print help");
    com_println("L val          LED on/off");
    com_println("b              reset to bootloader");
    com_println("addr, range in hex");
}

static void print_read(unsigned int addr, unsigned int range)
{    
    printf("%03X", addr);

    for (unsigned int byte_idx = 0; byte_idx < range; byte_idx++) {
        printf(" %02X", at89_read(addr + byte_idx));
    }
    printf("\r\n");
}

static void print_sysflash(unsigned int addr, unsigned int range)
{    
    printf("%03X ", addr);

    for (unsigned int byte_idx = 0; byte_idx < range; byte_idx++) {
        printf(" %02X", at89_read_sysflash(addr + byte_idx));
    }
    printf("\r\n");
}

static bool sig_check()
{
    if (!checking_sig) {
        return true;
    }

    uint8_t sig0 = at89_read_sig(0);
    uint8_t sig1 = at89_read_sig(1);
    uint8_t sig2 = at89_read_sig(2);

    // quick power sequencing can glitch to
    // ERROR: bad signature (02 51 FF), ignoring command.
    if (sig0 == 0x1E && sig1 == 0x51 && sig2 == 0xFF) {
        return true;
    }

    printf("ERROR: bad signature (%02X %02X %02X), ignoring command.\r\n",
            sig0, sig1, sig2);
    printf("Please make sure the target is inserted in the correct orientation.\r\n");
    return false;
}

static bool blank_check()
{
    //Update address in place by using backspace
    printf("Performing a blank-check... ");
    unsigned char data = 0;
    for (unsigned int addr = 0; addr < 0xFFF; addr++) {
        printf("%03X", addr);
        data = at89_read(addr);
        printf("\b\b\b");
        if (data != 0xFF) {
            printf("done\r\n");
            printf("%03X set to byte %02X\r\n", addr, data);
            printf("Result: not blank\r\n");
            return false;
        }
    }
    printf("done\r\n");
    printf("Result: blank\r\n");
    return true;
}

static void self_test()
{
    printf("Testing first 255 bytes...\r\n");
    at89_erase();
    com_println("");
    for(unsigned int addr = 0; addr < 0xff; addr++) {
        at89_write(addr, addr);
        com_println("");
    }
    print_read(0,0xFF);
    printf("Testing last 255 bytes...\r\n");
    for(unsigned int addr = 0xF00; addr <= 0xFFF; addr++) {
        at89_write(addr, addr - 0xF00);
        com_println("");
    }
    print_read(0xF00, 0xFF);
    printf("\r\nTesting last byte...\r\n");
    at89_write(0xFFF, 0);
    com_println("");
    print_read(0xFFF, 1);
    printf("\r\ndone.\r\n");
}

static void print_sig(void)
{
    uint8_t sig0 = at89_read_sig(0);
    uint8_t sig1 = at89_read_sig(1);
    uint8_t sig2 = at89_read_sig(2);

    printf("(0x30) Manufacturer: %02X\r\n", sig0);
    printf("(0x31) Model:        %02X\r\n", sig1);
    printf("(0x32) VPP Voltage:  %02X\r\n", sig2);
}

static inline void eval_command(char *cmd)
{
    unsigned char *cmd_t = strtok(cmd, " ");

    if (cmd_t == NULL) {
        return;
    }

    switch (cmd_t[0]) {
    case 'r':
    {
        if (!sig_check()) {
            break;
        }
        
        unsigned int addr  = xtoi(strtok(NULL, " "));
        unsigned int range = xtoi(strtok(NULL, " "));
        print_read(addr, range);
        break;
    }
        
    case 'w':
    {
        if (!sig_check()) {
            break;
        }
        
        unsigned int addr  = xtoi(strtok(NULL, " "));
        unsigned char data = xtoi(strtok(NULL, " "));
        at89_write(addr, data);
        break;
    }

    case 'R':
    {
        if (!sig_check()) {
            break;
        }
        
        unsigned int addr  = xtoi(strtok(NULL, " "));
        unsigned int range = xtoi(strtok(NULL, " "));
        print_sysflash(addr, range);
        break;
    }

    case 'l':
    {
        if (!sig_check()) {
            break;
        }
        
        unsigned char mode = atoi(strtok(NULL, " "));
        at89_lock(mode);
        break;
    }
        
    case 'e':
        if (!sig_check()) {
            break;
        }
        
        at89_erase();
        break;

    case 's':
        print_sig();
        break;

    case 'S':
        if (arg_bit()) {
            checking_sig = last_bit;
        }
        break;

    case 'T':
        if (!sig_check()) {
            break;
        }
        
        self_test();
        break;
        
    case 'B':
        if (!sig_check()) {
            break;
        }
        
        blank_check();
        break;

    case '?':
    case 'h':
        print_help();
        break;

    //LED on/off
    case 'L':
    {
        if (arg_bit()) {
            LED = last_bit;
        }
        break;
    }

    case 'b':
        stock_reset_to_bootloader();
        break;

    default:
        printf("ERROR: unknown command 0x%02X (%c)\r\n", cmd_t[0], cmd_t[0]);
        break;
    }
}

void mode_main(void) {
    vpp_dis();
    
     while(1) {
        eval_command(com_cmd_prompt());
    }
}

void interrupt high_priority isr()
{
    usb_service();
}
