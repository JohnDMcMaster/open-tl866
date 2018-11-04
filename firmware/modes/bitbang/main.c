/*
 * open-tl866 firmware
 * TODO: Add description
 */

#include <xc.h>

#include "../../stock_compat.h"
#include "../../mode.h"
#include "../../comlib.h"
#include "../../arglib.h"
#include "../../io.h"

static inline void print_help(void)
{
    com_println("open-tl866 (bitbang)");
    com_println("VPP");
    com_println("E val      VPP: enable and/or disable (VPP_DISABLE/VPP_ENABLE)");
    com_println("P val      VPP: set voltage enum (VPP_SET)");
    com_println("p val      VPP: set active pins (VPP_WRITE)");
    com_println("VDD");
    com_println("e val      VDD: enable and/or disable (VDD_DISABLE/VDD_ENABLE)");
    com_println("D val      VDD: set voltage enum (VDD_SET)");
    com_println("d val      VDD: set active pins (VDD_WRITE)");
    com_println("GND");
    com_println("g val      GND: set active pins (GND_WRITE)");
    com_println("I/O");
    com_println("t val      write ZIF tristate setting (ZIF_DIR)");
    com_println("T          read ZIF tristate setting (ZIF_DIR_READ)");
    com_println("z val      write ZIF pins (ZIF_WRITE)");
    com_println("Z          read ZIF pins (ZIF_READ)");
    com_println("Misc");
    com_println("l val      LED on/off (LED_ON/LED_OFF)");
    com_println("L          LED read state (LED_QUERY)");
    com_println("m val      pullup/pulldown (MYSTERY_ON/MYSTERY_OFF}");
    com_println("b          reset to bootloader (RESET_BOOTLOADER)");
}

static inline void eval_command(char * cmd)
{
    unsigned char * cmd_t = strtok(cmd, " ");

    switch (cmd_t[0]) {
    /*
    VPP
    */

    case 'E':
        if (arg_i()) {
            if (last_i) {
                vpp_en();
            } else {
                vpp_dis();
            }
        }
        break;

    case 'P':
        if (arg_i()) {
            vpp_val(last_i);
        }
        break;

    case 'p':
        if (arg_zif()) {
            set_vpp(last_zif);
        }
        break;


    /*
    VDD
    */

    case 'e':
        if (arg_i()) {
            if (last_i) {
                vdd_en();
            } else {
                vdd_dis();
            }
        }
        break;

    case 'D':
        if (arg_i()) {
            vdd_val(last_i);
        }
        break;

    case 'd':
        if (arg_zif()) {
            set_vdd(last_zif);
        }
        break;


    /*
    GND
    */

    case 'G':
        if (arg_zif()) {
            set_gnd(last_zif);
        }
        break;


    /*
    I/O
    */

    case 't':
        if (arg_zif()) {
            dir_write(last_zif);
        }
        break;

    case 'Z':
    {
        zif_bits_t zif = { 0x00 };
        zif_read(zif);
        print_zif_bits("Result", zif);
        break;
    }

    case 'T':
    {
        zif_bits_t zif = { 0x00 };
        dir_read(zif);
        print_zif_bits("Result", zif);
        break;
    }

    case 'z':
        if (arg_zif()) {
            zif_write(last_zif);
        }
        break;


    /*
    Misc
    */

    case 'l':
    {
        if (arg_bit()) {
            LED = last_bit;
        }
        break;
    }

    case 'L':
        printf("Result: %d\r\n", LED);
        break;

    case 'm':
        PUPD_PORT = arg_bit();
        break;

    case 'b':
        stock_reset_to_bootloader();
        break;

    case '?':
    case 'h':
        print_help();
        break;

    case 0:
        break;
    default:
        com_print("Error: Unknown command\r\n");
        break;
    }
}

void mode_main(void)
{
    while(1) {
        eval_command(com_cmd_prompt());
    }
}

