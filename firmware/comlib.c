#include "comlib.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

int echo = 1;
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
// str may be of any length, but it must be null-terminated
static inline void send_string_sync(uint8_t endpoint, const char *str)
{
    unsigned int len = strlen(str);
    unsigned num_bytes_to_send;
    char *in_buf = NULL;

    while (len > 0) {
        while (usb_in_endpoint_busy(endpoint))
            ;
        in_buf = (char *)usb_get_in_buffer(endpoint);
        num_bytes_to_send = MIN(len, 64);
        memcpy(in_buf, str, num_bytes_to_send);
        usb_send_in_buffer(endpoint, num_bytes_to_send);
        len -= num_bytes_to_send;
        str += num_bytes_to_send;
    }
}

static inline void send_char_sync(uint8_t endpoint, char c)
{
    char *in_buf = NULL;

    while (usb_in_endpoint_busy(endpoint))
        ;

    in_buf = (char *)usb_get_in_buffer(endpoint);
    in_buf[0] = c;

    usb_send_in_buffer(endpoint, 1);
}

static inline bool usb_ready()
{
    return usb_is_configured() && !usb_out_endpoint_halted(COM_ENDPOINT) &&
           usb_out_endpoint_has_data(COM_ENDPOINT);
}

// Read a line from USB input. Blocking.
unsigned char *com_readline()
{
    // TODO: Might be good to make a global struct to handle the command buffer
    static unsigned char cmd_buf[64];
    memset(cmd_buf, 0, sizeof(cmd_buf));
    int cmd_ptr = 0;

    while (1) {
        /* Handle data received from the host */
        if (usb_ready()) {

            const unsigned char *out_buf;

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

            if (echo) {
                printf(out_buf);

                // Temporary workaround buffer printing out previous char of
                // previous input characters after a certain length.
                // Real fix is to make sure all strings are properly null
                // terminated. TODO
                memset(out_buf, 0, 64);
            }

            if (!newline_found) {
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

void com_print(const char *str)
{
    send_string_sync(COM_ENDPOINT, str);
}

void com_println(const char *str)
{
    send_string_sync(COM_ENDPOINT, str);
    send_string_sync(COM_ENDPOINT, "\r\n");
}

// used by printf type functions
void putch(const unsigned char c)
{
    // TODO: Make a buffer and flush function to avoid sending one character
    // per USB packet.
    send_char_sync(COM_ENDPOINT, c);
}

char *com_cmd_prompt(void)
{
    char *cmd;

    printf("CMD> ");
    cmd = com_readline();
    com_println("");
    return cmd;
}
