/*
 * open-tl866 firmware
 * TODO: Add description
 */

#include "bitbang.h"
#include "../../stock_compat.h"
#include "../../mode.h"

static inline void handle_command(parse_result_t *res) {
    switch(res->cmd)
    {
        case ZIF_DIR:
            dir_write(res->arg);
            com_print("Ok 0\r\n");
            break;
        case ZIF_READ:
            {
                char str_ret[16] = {'O', 'k', ' ', '0', '0', '0',
                                    '0', '0', '0', '0', '0', '0',
                                    '0', '\r', '\n', '\0'};
                zif_bits_t zif = { 0x00 };
                zif_read(zif);
                hex_to_ascii(&str_ret[3], zif, 10, 5);
                com_print(str_ret);
            }
            break;
        case ZIF_DIR_READ:
            {
                char str_ret[16] = {'O', 'k', ' ', '0', '0', '0',
                                    '0', '0', '0', '0', '0', '0',
                                    '0', '\r', '\n', '\0'};
                zif_bits_t zif = { 0x00 };
                dir_read(zif);
                hex_to_ascii(&str_ret[3], zif, 10, 5);
                com_print(str_ret);
            }
            break;
        case ZIF_WRITE:
            zif_write(res->arg);
            com_print("Ok 0\r\n");
            break;
        case VDD_DISABLE:
            vdd_dis();
            com_print("Ok 0\r\n");
            break;
        case VDD_ENABLE:
            vdd_en();
            com_print("Ok 0\r\n");
            break;
        case VPP_DISABLE:
            vpp_dis();
            com_print("Ok 0\r\n");
            break;
        case VPP_ENABLE:
            vpp_en();
            com_print("Ok 0\r\n");
            break;
        case VDD_SET:
            vdd_val(res->arg[0]);
            com_print("Ok 0\r\n");
            break;
        case VPP_SET:
            vpp_val(res->arg[0]);
            com_print("Ok 0\r\n");
            break;
        case GND_WRITE:
            set_gnd(res->arg);
            com_print("Ok 0\r\n");
            break;
        case VDD_WRITE:
            set_vdd(res->arg);
            com_print("Ok 0\r\n");
            break;
        case VPP_WRITE:
            set_vpp(res->arg);
            com_print("Ok 0\r\n");
            break;
        case ECHO_ON:
            enable_echo();
            com_print("Ok 1\r\n");
            break;

        case ECHO_OFF:
            disable_echo();
            com_print("Ok 0\r\n");
            break;

        case LED_ON:
            LED = 1;
            com_print("Ok 0\r\n");
            break;

        case LED_OFF:
            LED = 0;
            com_print("Ok 0\r\n");
            break;

        case LED_QUERY:
            {
                const char * msg = LED ? "Ok 1\r\n" : "Ok 0\r\n";
                com_print(msg);
            }
            break;

        case MYSTERY_ON:
            MYSTERY = 1;
            com_print("Ok 1\r\n");
            break;

        case MYSTERY_OFF:
            MYSTERY = 0;
            com_print("Ok 0\r\n");
            break;

        case RESET_BOOTLOADER:
            stock_reset_to_bootloader();
            break;

        case INVALID:
        default:
            com_print("Error: Unknown command.\r\n");
            break;
    }
}

void mode_main(void)
{
    parse_result_t res;
    unsigned char * cmd;
    
    while (1) {
        cmd = com_readline();
        parse_ascii(cmd, &res);
        handle_command(&res);
    }
}
