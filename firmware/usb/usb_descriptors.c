/*
 * USB Descriptors file for open-tl866
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
 */

#include "usb_config.h"
#include "usb.h"
#include "usb_ch9.h"
#include "usb_cdc.h"

#include "../stock_compat.h"

#ifdef __C18
#define ROMPTR rom
#else
#define ROMPTR
#endif

/* Configuration Packet
 *
 * This packet contains a configuration descriptor, one or more interface
 * descriptors, class descriptors(optional), and endpoint descriptors for a
 * single configuration of the device.  This struct is specific to the
 * device, so the application will need to add any interfaces, classes and
 * endpoints it intends to use.  It is sent to the host in response to a
 * GET_DESCRIPTOR[CONFIGURATION] request.
 *
 * While Most devices will only have one configuration, a device can have as
 * many configurations as it needs.  To have more than one, simply make as
 * many of these structs as are required, one for each configuration.
 *
 * An instance of each configuration packet must be put in the
 * usb_application_config_descs[] array below (which is #defined in
 * usb_config.h) so that the USB stack can find it.
 *
 * See Chapter 9 of the USB specification from usb.org for details.
 *
 * It's worth noting that adding endpoints here does not automatically
 * enable them in the USB stack.  To use an endpoint, it must be declared
 * here and also in usb_config.h.
 */
struct configuration_1_packet {
	struct configuration_descriptor  config;

	/* Stock Compatibility Interface */
	struct interface_descriptor      stock_interface;
	struct endpoint_descriptor       stock_ep_out;
	struct endpoint_descriptor       stock_ep_in;

	struct interface_association_descriptor cdc_iad;

	/* CDC Class Interface */
	struct interface_descriptor      cdc_class_interface;
	struct cdc_functional_descriptor_header cdc_func_header;
	struct cdc_acm_functional_descriptor cdc_acm;
	struct cdc_union_functional_descriptor cdc_union;
	struct endpoint_descriptor       cdc_ep;

	/* CDC Data Interface */
	struct interface_descriptor      cdc_data_interface;
	struct endpoint_descriptor       data_ep_in;
	struct endpoint_descriptor       data_ep_out;

};


/* Device Descriptor
 *
 * Each device has a single device descriptor describing the device.  The
 * format is described in Chapter 9 of the USB specification from usb.org.
 * USB_DEVICE_DESCRIPTOR needs to be defined to the name of this object in
 * usb_config.h.  For more information, see USB_DEVICE_DESCRIPTOR in usb.h.
 */
const ROMPTR struct device_descriptor this_device_descriptor =
{
	sizeof(struct device_descriptor), // bLength
	DESC_DEVICE, // bDescriptorType
	0x0200, // 0x0200 = USB 2.0, 0x0110 = USB 1.1
	DEVICE_CLASS_MISC, // Device class
	0x02, /* Device Subclass. See the document entitled: "USB Interface
	         Association Descriptor Device Class Code and Use Model" */
	0x01, // Protocol. See document referenced above.
	EP_0_LEN, // bMaxPacketSize0
	0x04D8, // Vendor
	0xE11C, // Product
	0x0000, // device release (1.0)
	1, // Manufacturer
	2, // Product
	3, // Serial
	NUMBER_OF_CONFIGURATIONS // NumConfigurations
};

/* Configuration Packet Instance
 *
 * This is an instance of the configuration_packet struct containing all the
 * data describing a single configuration of this device.  It is wise to use
 * as much C here as possible, such as sizeof() operators, and #defines from
 * usb_config.h.  When stuff is wrong here, it can be difficult to track
 * down exactly why, so it's good to get the compiler to do as much of it
 * for you as it can.
 */
static const ROMPTR struct configuration_1_packet configuration_1 =
{
	{
	sizeof(struct configuration_descriptor),
	DESC_CONFIGURATION,
	sizeof(configuration_1), // wTotalLength (length of the whole packet)
	3, // bNumInterfaces
	1, // bConfigurationValue
	0, // iConfiguration (index of string descriptor)
	0b10000000,
	100/2,   // 100/2 indicates 100mA
	},


	/* Stock Compatibility Interface */
	{
	sizeof(struct interface_descriptor), // bLength
	DESC_INTERFACE,
	0, // InterfaceNumber
	0, // AlternateSetting
	2, // bNumEndpoints
	0, // bInterfaceClass
	0, // bInterfaceSubClass
	0, // bInterfaceProtocol
	4, // iInterface
	},

	/* Stock Compat Write (Endpoint 1 OUT) */
	{
	sizeof(struct endpoint_descriptor),
	DESC_ENDPOINT,
	0x01 | 0x00, // endpoint #1 0x00=OUT
	EP_BULK, // bmAttributes
	EP_1_OUT_LEN, // wMaxPacketSize
	1, // bInterval in ms.
	},

	/* Stock Compat Read (Endpoint 1 IN) */
	{
	sizeof(struct endpoint_descriptor),
	DESC_ENDPOINT,
	0x01 | 0x80, // endpoint #1 0x80=IN
	EP_BULK, // bmAttributes
	EP_1_IN_LEN, // wMaxPacketSize
	1, // bInterval in ms.
	},



	/* Interface Association Descriptor */
	{
	sizeof(struct interface_association_descriptor),
	DESC_INTERFACE_ASSOCIATION,
	1, /* bFirstInterface */
	2, /* bInterfaceCount */
	CDC_COMMUNICATION_INTERFACE_CLASS,
	CDC_COMMUNICATION_INTERFACE_CLASS_ACM_SUBCLASS,
	0, /* bFunctionProtocol */
	5, /* iFunction (string descriptor index) */
	},

	/* CDC Class Interface */
	{
	sizeof(struct interface_descriptor), // bLength;
	DESC_INTERFACE,
	0x1, // InterfaceNumber
	0x0, // AlternateSetting
	0x1, // bNumEndpoints
	CDC_COMMUNICATION_INTERFACE_CLASS, // bInterfaceClass
	CDC_COMMUNICATION_INTERFACE_CLASS_ACM_SUBCLASS, // bInterfaceSubclass
	0x00, // bInterfaceProtocol
	5, // iInterface
	},

	/* CDC Functional Descriptor Header */
	{
	sizeof(struct cdc_functional_descriptor_header),
	DESC_CS_INTERFACE,
	CDC_FUNCTIONAL_DESCRIPTOR_SUBTYPE_HEADER,
	0x0110, /* bcdCDC (version in BCD) */
	},

	/* CDC ACM Functional Descriptor */
	{
	sizeof(struct cdc_acm_functional_descriptor),
	DESC_CS_INTERFACE,
	CDC_FUNCTIONAL_DESCRIPTOR_SUBTYPE_ACM,
	/* bmCapabilities: Make sure to keep in sync with the actual
	 * capabilities (ie: which callbacks are defined). */
	CDC_ACM_CAPABILITY_LINE_CODINGS | CDC_ACM_CAPABILITY_SEND_BREAK,
	},

	/* CDC Union Functional Descriptor */
	{
	sizeof (struct cdc_union_functional_descriptor),
	DESC_CS_INTERFACE,
	CDC_FUNCTIONAL_DESCRIPTOR_SUBTYPE_UNION,
	1, /* bMasterInterface */
	2, /* bSlaveInterface0 */
	},

	/* CDC ACM Notification Endpoint (Endpoint 2 IN) */
	{
	sizeof(struct endpoint_descriptor),
	DESC_ENDPOINT,
	0x02 | 0x80, // endpoint #2 0x80=IN
	EP_INTERRUPT, // bmAttributes
	EP_2_IN_LEN, // wMaxPacketSize
	1, // bInterval in ms.
	},

	/* CDC Data Interface */
	{
	sizeof(struct interface_descriptor), // bLength;
	DESC_INTERFACE,
	0x2, // InterfaceNumber
	0x0, // AlternateSetting
	0x2, // bNumEndpoints
	CDC_DATA_INTERFACE_CLASS, // bInterfaceClass
	0, // bInterfaceSubclass (no subclass)
	CDC_DATA_INTERFACE_CLASS_PROTOCOL_NONE, // bInterfaceProtocol
	5, // iInterface
	},

	/* CDC Data IN Endpoint (Endpoint 3 IN) */
	{
	sizeof(struct endpoint_descriptor),
	DESC_ENDPOINT,
	0x03 | 0x80, // endpoint #3 0x80=IN
	EP_BULK, // bmAttributes
	EP_3_IN_LEN, // wMaxPacketSize
	1, // bInterval in ms.
	},

	/* CDC Data OUT Endpoint (Endpoint 3 OUT) */
	{
	sizeof(struct endpoint_descriptor),
	DESC_ENDPOINT,
	0x03 /*| 0x00*/, // endpoint #3 0x00=OUT
	EP_BULK, // bmAttributes
	EP_3_OUT_LEN, // wMaxPacketSize
	1, // bInterval in ms.
	},
};



/* String Descriptors
 *
 * String descriptors are optional. If strings are used, string #0 is
 * required, and must contain the language ID of the other strings.  See
 * Chapter 9 of the USB specification from usb.org for more info.
 *
 * Strings are UTF-16 Unicode, and are not NULL-terminated, hence the
 * unusual syntax.
 */

/* String index 0, only has one character in it, which is to be set to the
   language ID of the language which the other strings are in. */
static const ROMPTR struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t lang; } str00 = {
	sizeof(str00),
	DESC_STRING,
	0x0409 // US English
};

static const ROMPTR struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t chars[15]; } vendor_string = {
	sizeof(vendor_string),
	DESC_STRING,
	{'a','u','t','o','e','l','e','c','t','r','i','c','.','c','n'}
};

static const ROMPTR struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t chars[37]; } product_string = {
	sizeof(product_string),
	DESC_STRING,
	{'M','i','n','i','P','r','o',' ','T','L','8','6','6',' ','P','r','o','g','r','a','m','m','e','r',
	 ' ','[','O','p','e','n','-','T','L','8','6','6',']'
	}
};

static const ROMPTR struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t chars[17]; } stock_interface_string = {
	sizeof(stock_interface_string),
	DESC_STRING,
	{'B','o','o','t','l','o','a','d','e','r',' ','C','o','m','p','a','t'}
};

static const ROMPTR struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t chars[17]; } cdc_interface_string = {
	sizeof(cdc_interface_string),
	DESC_STRING,
	{'O','p','e','n','-','T','L','8','6','6',' ','S','e','r','i','a','l'}
};



/* Get String function
 *
 * This function is called by the USB stack to get a pointer to a string
 * descriptor.  If using strings, USB_STRING_DESCRIPTOR_FUNC must be defined
 * to the name of this function in usb_config.h.  See
 * USB_STRING_DESCRIPTOR_FUNC in usb.h for information about this function.
 * This is a function, and not simply a list or map, because it is useful,
 * and advisable, to have a serial number string which may be read from
 * EEPROM or somewhere that's not part of static program memory.
 */
int16_t usb_application_get_string(uint8_t string_number, const void **ptr)
{
	switch (string_number) {
	case 0:
		*ptr = &str00;
		return sizeof(str00);
	case 1:
		*ptr = &vendor_string;
		return sizeof(vendor_string);
	case 2:
		*ptr = &product_string;
		return sizeof(product_string);
	case 3:
		*ptr = &serial_block.serial;
		return sizeof(serial_block.serial);
	case 4:
		*ptr = &stock_interface_string;
		return sizeof(stock_interface_string);
	case 5:
		*ptr = &cdc_interface_string;
		return sizeof(cdc_interface_string);
	default:
		return -1;
	}
}

/* Configuration Descriptor List
 *
 * This is the list of pointters to the device's configuration descriptors.
 * The USB stack will read this array looking for descriptors which are
 * requsted from the host.  USB_CONFIG_DESCRIPTOR_MAP must be defined to the
 * name of this array in usb_config.h.  See USB_CONFIG_DESCRIPTOR_MAP in
 * usb.h for information about this array.  The order of the descriptors is
 * not important, as the USB stack reads bConfigurationValue for each
 * descriptor to know its index.  Make sure NUMBER_OF_CONFIGURATIONS in
 * usb_config.h matches the number of descriptors in this array.
 */
const struct configuration_descriptor *usb_application_config_descs[] =
{
	(struct configuration_descriptor*) &configuration_1,
};
STATIC_SIZE_CHECK_EQUAL(USB_ARRAYLEN(USB_CONFIG_DESCRIPTOR_MAP), NUMBER_OF_CONFIGURATIONS);
STATIC_SIZE_CHECK_EQUAL(sizeof(USB_DEVICE_DESCRIPTOR), 18);

