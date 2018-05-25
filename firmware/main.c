/*
 * open-tl866 firmware
 * TODO: Add description
 */

#include "usb.h"
#include "usb/usb_callbacks.c"

#include <xc.h>
#include <string.h>

#include "io.h"
#include "parse.h"

const char * led_status[2] = {
    "LED is off.\n",
    "LED is on.\n"
};
int echo = 0;

static void send_string_sync(uint8_t endpoint, const char *str)
{
    char *in_buf = (char*) usb_get_in_buffer(endpoint);

    while (usb_in_endpoint_busy(endpoint))
        ;

    strcpy(in_buf, str);
    /* Hack: Get the length from strlen(). This is inefficient, but it's
     * just a demo. strlen()'s return excludes the terminating NULL. */
    usb_send_in_buffer(endpoint, strlen(in_buf));
}

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

static inline void handle_command(parse_result_t *res) {
    switch(res->cmd)
    {
        case ZIF_DIR:
            dir_write(res->arg);
            send_string_sync(2, "Ok 0\r\n");
            break;
        case ZIF_READ:
            {
                char str_ret[16] = {'O', 'k', ' ', '0', '0', '0',
                                    '0', '0', '0', '0', '0', '0',
                                    '0', '\r', '\n', '\0'};
                zif_bits_t zif = { 0x00 };
                zif_read(zif);
                hex_to_ascii(&str_ret[3], zif, 10, 5);
                send_string_sync(2, str_ret);
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
                send_string_sync(2, str_ret);
            }
            break;
        case ZIF_WRITE:
            zif_write(res->arg);
            send_string_sync(2, "Ok 0\r\n");
            break;
        case VDD_DISABLE:
            vdd_dis();
            send_string_sync(2, "Ok 0\r\n");
            break;
        case VDD_ENABLE:
            vdd_en();
            send_string_sync(2, "Ok 0\r\n");
            break;
        case VPP_DISABLE:
            vpp_dis();
            send_string_sync(2, "Ok 0\r\n");
            break;
        case VPP_ENABLE:
            vpp_en();
            send_string_sync(2, "Ok 0\r\n");
            break;
        case VDD_SET:
            vdd_val(res->arg[0]);
            send_string_sync(2, "Ok 0\r\n");
            break;
        case VPP_SET:
            vpp_val(res->arg[0]);
            send_string_sync(2, "Ok 0\r\n");
            break;
        case GND_WRITE:
            set_gnd(res->arg);
            send_string_sync(2, "Ok 0\r\n");
            break;
        case VDD_WRITE:
            set_vdd(res->arg);
            send_string_sync(2, "Ok 0\r\n");
            break;
        case VPP_WRITE:
            set_vpp(res->arg);
            send_string_sync(2, "Ok 0\r\n");
            break;
        case ECHO_ON:
            echo = 1;
            send_string_sync(2, "Ok 1\r\n");
            break;

        case ECHO_OFF:
            echo = 0;
            send_string_sync(2, "Ok 0\r\n");
            break;

        case LED_ON:
            LED = 1;
            send_string_sync(2, "Ok 0\r\n");
            break;

        case LED_OFF:
            LED = 0;
            send_string_sync(2, "Ok 0\r\n");
            break;

        case LED_QUERY:
            {
                const char * msg = LED ? "Ok 1\r\n" : "Ok 0\r\n";
                send_string_sync(2, msg);
            }
            break;

        case MYSTERY_ON:
            MYSTERY = 1;
            send_string_sync(2, "Ok 1\r\n");
            break;

        case MYSTERY_OFF:
            MYSTERY = 0;
            send_string_sync(2, "Ok 0\r\n");
            break;

        case INVALID:
        default:
            send_string_sync(2, "Err 2\r\n");
            break;
    }
}

int main(void)
{
    init();

    uint8_t char_to_send = 'A';
    bool send = true;
    bool loopback = false;

    unsigned char cmd_buf[64];
    int cmd_ptr = 0;

    while (1) {
        /* Handle data received from the host */
        if (usb_is_configured() &&
            !usb_out_endpoint_halted(2) &&
            usb_out_endpoint_has_data(2)) {

            const unsigned char *out_buf;
            size_t out_buf_len;
            int newline_found = 0;
            parse_result_t res;

            /* Check for an empty transaction. */
            out_buf_len = usb_get_out_buffer(2, &out_buf);
            if (out_buf_len <= 0)
                goto empty;

            /* If copying would overflow, discard whole command and error. */
            if(cmd_ptr + out_buf_len > 63)
            {
                send_string_sync(2, "Err 1\r\n");
                cmd_ptr = 0;
                goto empty;
            }

            /* Look for a full line. */
            memcpy(cmd_buf + cmd_ptr, out_buf, out_buf_len);
            for(int i = 0; i < out_buf_len; i++)
            {
                if(cmd_buf[cmd_ptr + i] == '\n' || cmd_buf[cmd_ptr + i] == '\r')
                {
                    newline_found = 1;
                    cmd_ptr = 0;
                    break;
                }
            }

            if(echo)
            {
                send_string_sync(2, out_buf);
            }

            if(!newline_found)
            {
                cmd_ptr += out_buf_len;
                goto empty;
            }

            parse_ascii(cmd_buf, &res);
            handle_command(&res);
            cmd_ptr = 0;

empty:
            usb_arm_out_endpoint(2);
        }
    }

    return 0;
}

void interrupt high_priority isr()
{
    usb_service();
}