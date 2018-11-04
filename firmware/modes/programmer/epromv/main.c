//27C256

#include <xc.h>

#include "system.h"

//#include "epromv.h"
#include "../../../mode.h"
#include "../../../comlib.h"
#include "../../../stock_compat.h"

#define EZZIF_DIP28
#include "ezzif.h"

int main_debug = 0;

static const char ADDR_BUS[] = {
    //LSB (A0-A7)
    10, 9, 8, 7, 6, 5, 4, 3,
    //MSB (A8-A14)
    25, 24, 21, 23, 2, 26, 27,
};
static const char DATA_BUS[] = {11, 12, 13, 15, 16, 17, 18, 19};

static void prompt_msg(const char *msg) {
    if (main_debug) {
        ezzif_print_debug();
    }
    com_println(msg);
    com_readline();
}

static void prompt_enter(void) {
    prompt_msg("Press enter to continue");
}

static inline void print_help(void)
{
    com_println("open-tl866 (eprom-v)");
    com_println("r addr range   Read from target");
    com_println("h              Print help");
    com_println("V              Print version(s)");
    com_println("b              reset to bootloader");
}

static void dev_addr(int n) {
    ezzif_bus_w(ADDR_BUS, sizeof(ADDR_BUS), n);
}

static void dev_init(void) {
    ezzif_reset();

    ezzif_vdd(28, VDD_51);  //VCC
    ezzif_vdd(1, VDD_51);   //VPP = VCC
    ezzif_gnd(14);          //VSS

    ezzif_io(20, 0, 0);     // CEn
    ezzif_io(22, 0, 0);     // OEn

    //Address bus output to 0
    dev_addr(0);
    ezzif_bus_dir(ADDR_BUS, sizeof(ADDR_BUS), 0);
}

static unsigned char read_byte(unsigned int addr)
{
    ezzif_bus_w(ADDR_BUS, sizeof(ADDR_BUS), addr);
    __delay_ms(1);
    return ezzif_bus_r(DATA_BUS, sizeof(DATA_BUS));
}

static void eprom_read(unsigned int addr, unsigned int range)
{
    printf("%03X ", addr);
    dev_init();
    
    if (!range) { range = 1; } else {com_println("");}
    for (unsigned int byte_idx = 0; byte_idx < range; byte_idx++) {
        printf("%02X ", read_byte(addr + byte_idx));
    }
    printf("\r\n");

    ezzif_reset();
}

static inline void eval_command(unsigned char * cmd)
{
    unsigned char * cmd_t = strtok(cmd, " ");
    switch (cmd_t[0]) {
    case 'r':
    {
        //unsigned int addr  = xtoi(strtok(NULL, " "));
        //unsigned int range = xtoi(strtok(NULL, " "));
        unsigned int addr  = 0;
        unsigned int range = 0x20;
        eprom_read(addr, range);
        break;
    }

    case '?':
    case 'h':
        print_help();
        break;
    case 'b':
        stock_reset_to_bootloader();
        break;
    default:
        printf("Error: Unknown command.");
    }
}

void mode_main(void) {
    ezzif_reset();
    
    while(1) {
        eval_command(com_cmd_prompt());
    }
}

