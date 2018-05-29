/*
 * USB CDC ACM Configuration for open-tl866
 *
 * This file may be used by anyone for any purpose and may be used as a
 * starting point making your own application using M-Stack.
 *
 * It is worth noting that M-Stack itself is not under the same license
 * as this file.
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
 * 2013-04-08
 */

#ifndef USB_CONFIG_H__
#define USB_CONFIG_H__

/* Number of endpoint numbers besides endpoint zero. It's worth noting that
   and endpoint NUMBER does not completely describe an endpoint, but the
   along with the DIRECTION does (eg: EP 1 IN).  The #define below turns on
   BOTH IN and OUT endpoints for endpoint numbers (besides zero) up to the
   value specified.  For example, setting NUM_ENDPOINT_NUMBERS to 2 will
   activate endpoints EP 1 IN, EP 1 OUT, EP 2 IN, EP 2 OUT.  */
#define NUM_ENDPOINT_NUMBERS 2

/* Only 8, 16, 32 and 64 are supported for endpoint zero length. */
#define EP_0_LEN 8

#define EP_1_OUT_LEN 1
#define EP_1_IN_LEN 10 /* May need to be longer, depending
                        * on the notifications you support. */
 /* The code in the demo app assumes that EP2 IN and OUT are the same length */
#define EP_2_LEN 64
#define EP_2_OUT_LEN EP_2_LEN
#define EP_2_IN_LEN EP_2_LEN

#define NUMBER_OF_CONFIGURATIONS 1

/* Ping-pong buffering mode. Valid values are:
	PPB_NONE         - Do not ping-pong any endpoints
	PPB_EPO_OUT_ONLY - Ping-pong only endpoint 0 OUT
	PPB_ALL          - Ping-pong all endpoints
	PPB_EPN_ONLY     - Ping-pong all endpoints except 0
*/
#ifdef __PIC32MX__
	/* PIC32MX only supports PPB_ALL */
	#define PPB_MODE PPB_ALL
#else
	#define PPB_MODE PPB_NONE
#endif

/* Comment the following line to use polling USB operation. When using polling,
   You are responsible for calling usb_service() periodically from your
   application. */
#define USB_USE_INTERRUPTS

/* Uncomment if you have a composite device which has multiple different types
 * of device classes. For example a device which has HID+CDC or
 * HID+VendorDefined, but not a device which has multiple of the same class
 * (such as HID+HID). Device class implementations have additional requirements
 * for multi-class devices. See the documentation for each device class for
 * details. */
//#define MULTI_CLASS_DEVICE


/* Objects from usb_descriptors.c */
#define USB_DEVICE_DESCRIPTOR this_device_descriptor
#define USB_CONFIG_DESCRIPTOR_MAP usb_application_config_descs
#define USB_STRING_DESCRIPTOR_FUNC usb_application_get_string

/* CDC Configuration functions. See usb_cdc.h for documentation. */
#define CDC_SEND_ENCAPSULATED_COMMAND_CALLBACK app_send_encapsulated_command
#define CDC_GET_ENCAPSULATED_RESPONSE_CALLBACK app_get_encapsulated_response
#define CDC_SET_COMM_FEATURE_CALLBACK app_set_comm_feature_callback
#define CDC_CLEAR_COMM_FEATURE_CALLBACK app_clear_comm_feature_callback
#define CDC_GET_COMM_FEATURE_CALLBACK app_get_comm_feature_callback
#define CDC_SET_LINE_CODING_CALLBACK app_set_line_coding_callback
#define CDC_GET_LINE_CODING_CALLBACK app_get_line_coding_callback
#define CDC_SET_CONTROL_LINE_STATE_CALLBACK app_set_control_line_state_callback
#define CDC_SEND_BREAK_CALLBACK app_send_break_callback

#endif /* USB_CONFIG_H__ */
