#include "comlib.h"

int echo = 0;
unsigned comblib_drops = 0;

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
    unsigned int len = strlen(str);
    char *in_buf = NULL;

    if (len > 64) {
        comblib_drops += len - 64;
        len = 64;
    }

    while (usb_in_endpoint_busy(endpoint));

    in_buf = (char*) usb_get_in_buffer(endpoint);
    strncpy(in_buf, str, len);

    // TODO: get deterministic lens at compile time. Add len argument to
    // this function, so non-deterministic lengths are calculated before
    // entry.

    /* Hack: Get the length from strlen(). This is inefficient, but it's
     * just a demo. strlen()'s return excludes the terminating NULL. */
    usb_send_in_buffer(endpoint, len);
}

static inline void send_char_sync(uint8_t endpoint, char c)
{
    char *in_buf = NULL;

    while (usb_in_endpoint_busy(endpoint));

    in_buf = (char*) usb_get_in_buffer(endpoint);
    in_buf[0] = c;

    usb_send_in_buffer(endpoint, 1);
}


static inline bool usb_ready()
{
    return usb_is_configured() &&
          !usb_out_endpoint_halted(COM_ENDPOINT) &&
           usb_out_endpoint_has_data(COM_ENDPOINT);
}

// Read a line from USB input. Blocking.
unsigned char * com_readline()
{
    // TODO: Might be good to make a global struct to handle the command buffer
    static unsigned char cmd_buf[64];
    memset(cmd_buf, 0, sizeof(cmd_buf));
    int cmd_ptr = 0;
    
    while(1) {
        /* Handle data received from the host */
        if (usb_ready()) {

            const unsigned char * out_buf;
            
            uint8_t out_buf_len;
            int newline_found = 0;

            /* Check for an empty transaction. */
            out_buf_len = usb_get_out_buffer(COM_ENDPOINT, &out_buf);
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
                    cmd_buf[cmd_ptr + i] = 0;
                    newline_found = 1;
                    cmd_ptr = 0;
                    break;
                }
            }

            if(echo) {
                printf(out_buf);
                
                // Temporary workaround buffer printing out previous char of
                // previous input characters after a certain length.
                // Real fix is to make sure all strings are properly null
                // terminated. TODO
                memset(out_buf, 0, 64);
            }

            if(!newline_found) {
                cmd_ptr += out_buf_len;
                goto empty;
            }

            cmd_ptr = 0;
            
            usb_arm_out_endpoint(COM_ENDPOINT);
            
            return cmd_buf;

            // Jump here when encountering empty string
            empty:
                usb_arm_out_endpoint(COM_ENDPOINT);
        }
    }
    return NULL;
}

// TODO: Add logic to ensure string fits in EP_2_OUT_LEN and split string
// into multiple USB packets if exceeded
void com_print(const char * str)
{
    send_string_sync(COM_ENDPOINT, str);
}

void com_println(const char * str)
{
    // TODO: Append to string instead, and split into multiple packets
    // if necessary as per previous TODO
    send_string_sync(COM_ENDPOINT, str);
    send_string_sync(COM_ENDPOINT, "\r\n");
}

//used by printf type functions
void putch(const unsigned char c)
{
    // TODO: Make a buffer and flush function to avoid sending one character
    // per USB packet.
    send_char_sync(COM_ENDPOINT, c);
}

char *com_cmd_prompt(void) {
    char *cmd;

    printf("\r\nCMD> ");
    cmd = com_readline();
    com_println("");
    return cmd;
}

