/*
 * USB CDC-ACM Demo
 *
 * This file may be used by anyone for any purpose and may be used as a
 * starting point making your own application using M-Stack.
 *
 * It is worth noting that M-Stack itself is not under the same license as
 * this file.
 *
 * M-Stack is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  For details, see sections 7, 8, and 9
 * of the Apache License, version 2.0 which apply to this file.  If you have
 * purchased a commercial license for this software from Signal 11 Software,
 * your commerical license superceeds the information in this header.
 *
 * Alan Ott
 * Signal 11 Software
 * 2014-05-12
 */

#include "usb.h"
#include <xc.h>
#include <string.h>
#include "usb_config.h"
#include "usb_ch9.h"
#include "usb_cdc.h"

#include "io.h"

const char * led_status[2] = {
    "LED is off.\n",
    "LED is on.\n"
};


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

int main(void)
{
    unsigned int pll_startup = 600;
	OSCTUNEbits.PLLEN = 1;
	while (pll_startup--);
    
    INTCONbits.PEIE = 1;
	INTCONbits.GIE = 1;
    
    PORTA = 0x00;
    TRISA = 0x00; // RA5-RA0: LE4, OE_VDD, LE5, LE2, LE7, LE3
    
    PORTB = 0x00;
    TRISB = 0xc1; // By default, keep RB7/RB6 as input/shared w/ ICSP header.
                  // RB1: Controls resistors on P15-P24. P16/P21 act especially weird.
                  // RB0: Input that detects that Vpp/Vdd voltage is okay.
    
    PORTC = 0x00;
    TRISC = 0x00; // RC1-RC0: ZIF Pin 20 GND driver enable, LED
    
    PORTD = 0x00; // All attached to ZIF
    
    PORTE = 0x00; // All attached to ZIF
    
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

    
    for(int i = 0; i < 2; i++)
    {
        write_latch(i, 0xff);
    }

    // Disable all pin drivers for initial "known" state.
    OE_VPP = 1;
    OE_VDD = 1;
    
    for(int i = 0; i < 8; i++)
    {
        write_latch(i, 0x00);
    }

    OE_VPP = 0;
    OE_VDD = 0;
    
	usb_init();

	uint8_t char_to_send = 'A';
	bool send = true;
	bool loopback = false;

	while (1) {
		/* Handle data received from the host */
		if (usb_is_configured() &&
		    !usb_out_endpoint_halted(2) &&
		    usb_out_endpoint_has_data(2)) {
			const unsigned char *out_buf;
			size_t out_buf_len;
			int i;

			/* Check for an empty transaction. */
			out_buf_len = usb_get_out_buffer(2, &out_buf);
			if (out_buf_len <= 0)
				goto empty;

            /* Scan for commands if not in loopback or
             * send mode.
             *
             * This is a hack. One should really scan the
             * entire string. In this case, since this
             * is a demo, assume that the user is using
             * a terminal program and typing the input,
             * all but ensuring the data will come in
             * single-character transactions. */
            if (out_buf[0] == 'h' || out_buf[0] == '?') {
                /* Show help.
                 * Make sure to not try to send more
                 * than 63 bytes of data in one
                 * transaction */
                send_string_sync(2,
                    "\r\nHelp:\r\n"
                    "\to: Turn LED off\r\n"
                    "\tl: Turn LED on\r\n");
                send_string_sync(2,
                    "\tq: Query LED status\r\n"
                    "\th: help\r\n");
            }
            else if (out_buf[0] == 'o') {
                PORTCbits.RC0 = 0;
            }
            else if (out_buf[0] == 'l') {
                PORTCbits.RC0 = 1;
            }
            else if (out_buf[0] == 'q') {
                int val = PORTCbits.RC0;         
                send_string_sync(2, led_status[val]);
            }
empty:
			usb_arm_out_endpoint(2);
		}
	}

	return 0;
}

/* Callbacks. These function names are set in usb_config.h. */
void app_set_configuration_callback(uint8_t configuration)
{

}

uint16_t app_get_device_status_callback()
{
	return 0x0000;
}

void app_endpoint_halt_callback(uint8_t endpoint, bool halted)
{

}

int8_t app_set_interface_callback(uint8_t interface, uint8_t alt_setting)
{
	return 0;
}

int8_t app_get_interface_callback(uint8_t interface)
{
	return 0;
}

void app_out_transaction_callback(uint8_t endpoint)
{

}

void app_in_transaction_complete_callback(uint8_t endpoint)
{

}

int8_t app_unknown_setup_request_callback(const struct setup_packet *setup)
{
	/* To use the CDC device class, have a handler for unknown setup
	 * requests and call process_cdc_setup_request() (as shown here),
	 * which will check if the setup request is CDC-related, and will
	 * call the CDC application callbacks defined in usb_cdc.h. For
	 * composite devices containing other device classes, make sure
	 * MULTI_CLASS_DEVICE is defined in usb_config.h and call all
	 * appropriate device class setup request functions here.
	 */
	return process_cdc_setup_request(setup);
}

int16_t app_unknown_get_descriptor_callback(const struct setup_packet *pkt, const void **descriptor)
{
	return -1;
}

void app_start_of_frame_callback(void)
{

}

void app_usb_reset_callback(void)
{

}

/* CDC Callbacks. See usb_cdc.h for documentation. */

int8_t app_send_encapsulated_command(uint8_t interface, uint16_t length)
{
	return -1;
}

int16_t app_get_encapsulated_response(uint8_t interface,
                                      uint16_t length, const void **report,
                                      usb_ep0_data_stage_callback *callback,
                                      void **context)
{
	return -1;
}

int8_t app_set_comm_feature_callback(uint8_t interface,
                                     bool idle_setting,
                                     bool data_multiplexed_state)
{
	return -1;
}

int8_t app_clear_comm_feature_callback(uint8_t interface,
                                       bool idle_setting,
                                       bool data_multiplexed_state)
{
	return -1;
}

int8_t app_get_comm_feature_callback(uint8_t interface,
                                     bool *idle_setting,
                                     bool *data_multiplexed_state)
{
	return -1;
}

static struct cdc_line_coding line_coding =
{
	115200,
	CDC_CHAR_FORMAT_1_STOP_BIT,
	CDC_PARITY_NONE,
	8,
};

int8_t app_set_line_coding_callback(uint8_t interface,
                                    const struct cdc_line_coding *coding)
{
	line_coding = *coding;
	return 0;
}

int8_t app_get_line_coding_callback(uint8_t interface,
                                    struct cdc_line_coding *coding)
{
	/* This is where baud rate, data, stop, and parity bits are set. */
	*coding = line_coding;
	return 0;
}

int8_t app_set_control_line_state_callback(uint8_t interface,
                                           bool dtr, bool dts)
{
	return 0;
}

int8_t app_send_break_callback(uint8_t interface, uint16_t duration)
{
	return 0;
}


void interrupt high_priority isr()
{
	usb_service();
}
