#include "comlib.h"

int echo = 0;

inline void enable_echo()
{
    echo = 1;
}

inline void disable_echo()
{
    echo = 0;
}

// Copy to the USB buffer once it's ready.
//
// This comes from the m-stack demo here:
// https://github.com/signal11/m-stack/blob/master/apps/cdc_acm/main.c
static inline void send_string_sync(uint8_t endpoint, const char *str)
{
    char *in_buf = (char*) usb_get_in_buffer(endpoint);

    while (usb_in_endpoint_busy(endpoint));

    strcpy(in_buf, str);

    // TODO: get deterministic lens at compile time. Add len argument to
    // this function, so non-deterministic lengths are calculated before
    // entry.

    /* Hack: Get the length from strlen(). This is inefficient, but it's
     * just a demo. strlen()'s return excludes the terminating NULL. */
    usb_send_in_buffer(endpoint, strlen(in_buf));
}

static inline void send_char_sync(uint8_t endpoint, const char *str)
{
    char *in_buf = (char*) usb_get_in_buffer(endpoint);

    while (usb_in_endpoint_busy(endpoint));

    strcpy(in_buf, str);

    usb_send_in_buffer(endpoint, 1);
}


static inline bool usb_ready()
{
    return usb_is_configured() &&
          !usb_out_endpoint_halted(2) &&
           usb_out_endpoint_has_data(2);
}

// Read a line from USB input. Blocking.
unsigned char * com_readline()
{
    // TODO: Might be good to make a global struct to handle the command buffer
    static unsigned char cmd_buf[64];
    memset(cmd_buf, 0, 64);
    int cmd_ptr = 0;
    
    while(1) {
        /* Handle data received from the host */
        if (usb_ready()) {

            const unsigned char * out_buf;
            
            size_t out_buf_len;
            int newline_found = 0;

            /* Check for an empty transaction. */
            out_buf_len = usb_get_out_buffer(2, &out_buf);
            if (out_buf_len <= 0) {
                goto empty;
            }

            /* If copying would overflow, discard whole command and error. */
            if (cmd_ptr + out_buf_len > 63) {
                com_print("Error: Command buffer exceeded.\r\n");
                cmd_ptr = 0;
                goto empty;
            }

            /* Look for a full line. */
            memcpy(cmd_buf + cmd_ptr, out_buf, out_buf_len);
            for (int i = 0; i < out_buf_len; i++) {
                if (cmd_buf[cmd_ptr + i] == '\n' ||
                    cmd_buf[cmd_ptr + i] == '\r') {
                    newline_found = 1;
                    cmd_ptr = 0;
                    break;
                }
            }

            if(echo) {
                com_print(out_buf);
            }

            if(!newline_found) {
                cmd_ptr += out_buf_len;
                goto empty;
            }

            cmd_ptr = 0;
            out_buf = 0;
            
            usb_arm_out_endpoint(2);
            
            return cmd_buf;

            // Jump here when encountering empty string
            empty:
                usb_arm_out_endpoint(2);
        }
    }
}

// TODO: Add logic to ensure string fits in EP_2_OUT_LEN and split string
// into multiple USB packets if exceeded
void com_print(const char * str)
{
   send_string_sync(2, str);
}

void com_println(const char * str)
{
    // TODO: Append to string instead, and split into multiple packets
    // if necessary as per previous TODO
    send_string_sync(2, str);
    send_string_sync(2, "\r\n");
}
void putch(const unsigned char c)
{
    send_char_sync(2, &c);
}