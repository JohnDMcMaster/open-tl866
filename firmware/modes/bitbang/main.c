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
    com_println(
    "open-tl866 (bitbang)\r\n"
    "VPP\r\n"
    "E val      VPP: enable or disable\r\n"
    "           1 = enable, 0 = disable\r\n"
    "V val      VPP: set voltage enum\r\n"
    "           val in range [0,7]\r\n"
    "p val      VPP: set active pins\r\n"
    "           val must be 10 hex digits\r\n"
    "           LSB is ZIF pin 1\r\n"
    "VDD\r\n"
    "e val      VDD: enable or disable\r\n"
    "           1 = enable, 0 = disable\r\n"
    "v val      VDD: set voltage enum\r\n"
    "           val in range [0,7]\r\n"
    "d val      VDD: set active pins\r\n"
    "           val must be 10 hex digits\r\n"
    "           LSB is ZIF pin 1\r\n"
    "GND\r\n"
    "g val      GND: set active pins (GND_WRITE)\r\n"
    "           val must be 10 hex digits\r\n"
    "           LSB is ZIF pin 1\r\n"
    "           NOTE: VDD must be enabled for GND to work\r\n"
    "I/O\r\n"
    "t val      I/O: set ZIF tristate setting\r\n"
    "           1 = pin is driven, 0 = High Z\r\n"
    "T          I/O: get ZIF tristate setting\r\n"
    "           1 = pin is driven, 0 = High Z\r\n"
    "z val      I/O: set ZIF pins (ZIF_WRITE)\r\n"
    "           val must be 10 hex digits\r\n"
    "           LSB is ZIF pin 1\r\n"
    "Z          I/O: get ZIF pins (ZIF_READ)\r\n"
    "           LSB is ZIF pin 1\r\n"
    "Misc\r\n"
    "L val      LED on/off\r\n"
    "           1 = on, 0 = off\r\n"
    "m z val    Set pullup/pulldown\r\n"
    "s          Print misc status\r\n"
    "i          Re-initialize\r\n"
    "b          Reset to bootloader\r\n");
}

static inline void eval_command(char *cmd)
{
    unsigned char *cmd_t = strtok(cmd, " ");

    if (cmd_t == NULL) {
        return;
    }

    switch (cmd_t[0]) {
    /*
    VPP
    */

    //VPP enable/disable
    case 'E':
        if (arg_i()) {
            if (last_i) {
                vpp_en();
            } else {
                vpp_dis();
            }
        }
        break;

    //VPP voltage value
    case 'V':
        if (arg_i()) {
            vpp_val(last_i);
        }
        break;

    //Set VPP pins
    case 'p':
        if (arg_zif()) {
            set_vpp(last_zif);
        }
        break;

    /*
    //Get VPP pins
    case 'p':
    {
        zif_bits_t zif = { 0x00 };
        get_vpp(zif);
        print_zif_bits("Result", zif);
        break;
    }
    */


    /*
    VDD
    */

    //VDD enable/disable
    case 'e':
        if (arg_i()) {
            if (last_i) {
                vdd_en();
            } else {
                vdd_dis();
            }
        }
        break;

    //VDD voltage value
    case 'v':
        if (arg_i()) {
            vdd_val(last_i);
        }
        break;

    //Set VDD pins
    case 'd':
        if (arg_zif()) {
            set_vdd(last_zif);
        }
        break;

    /*
    //Get VDD pins
    case 'D':
    {
        zif_bits_t zif = { 0x00 };
        get_vdd(zif);
        print_zif_bits("Result", zif);
        break;
    }
    */

    /*
    GND
    */

    //Set GND pins
    case 'g':
        if (arg_zif()) {
            set_gnd(last_zif);
        }
        break;

    /*
    //Get GND pins
    case 'G':
    {
        zif_bits_t zif = { 0x00 };
        get_gnd(zif);
        print_zif_bits("Result", zif);
        break;
    }
    */


    /*
    I/O
    */

    //Set I/O tristate
    case 't':
        if (arg_zif()) {
            dir_write(last_zif);
        }
        break;

    //Get I/O tristate
    case 'T':
    {
        zif_bits_t zif = { 0x00 };
        dir_read(zif);
        print_zif_bits("Result", zif);
        break;
    }

    //Write ZIF socket
    case 'z':
        if (arg_zif()) {
            zif_write(last_zif);
        }
        break;

    //Read ZIF socket
    case 'Z':
    {
        zif_bits_t zif = { 0x00 };
        zif_read(zif);
        print_zif_bits("Result", zif);
        break;
    }

    /*
    Misc
    */

    //LED on/off
    case 'L':
    {
        if (arg_bit()) {
            LED = last_bit;
        }
        break;
    }

    //Pullup/pulldown on/off
    case 'm': {
        if (arg_bit()) {
            int tristate = last_bit;
            if (arg_bit()) {
                pupd(tristate, last_bit);
            }
        }
        break;
    }

    //Misc status
    case 's':
        printf("Result nVPP_EN:%u nVDD_EN:%u LED:%u PUPD:Z%uV%u\r\n",
                nOE_VPP, nOE_VDD, LED, PUPD_TRIS, PUPD_PORT);
        break;

    //Re-initialize
    case 'i':
        io_init();
        break;

    //Help
    case '?':
    case 'h':
        print_help();
        break;

    //Bootloader
    case 'b':
        stock_reset_to_bootloader();
        break;

    //WTF
    default:
        printf("ERROR: unknown command 0x%02X (%c)\r\n", cmd_t[0], cmd_t[0]);
        break;
    }
}

void mode_main(void)
{
    while(1) {
        eval_command(com_cmd_prompt());
    }
}

void interrupt high_priority isr()
{
    usb_service();
}
