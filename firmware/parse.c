#include <string.h>

#include "parse.h"

typedef struct cmd_map
{
    char name[2];
    cmd_t cmd;
} cmd_map_t;

static cmd_map_t valid_cmds[] = {
    { "dd", VDD_DISABLE },
    { "de", VDD_ENABLE },
    { "ds", VDD_SET },
    { "dw", VDD_WRITE },
    { "ee", ECHO_ON },
    { "eo", ECHO_OFF },
    { "gw", GND_WRITE },
    { "ll", LED_ON },
    { "lo", LED_OFF },
    { "mm", MYSTERY_ON},
    { "mo", MYSTERY_OFF},
    { "lq", LED_QUERY },
    { "pd", VPP_DISABLE },
    { "pe", VPP_ENABLE },
    { "ps", VPP_SET },
    { "pw", VPP_WRITE },
    { "zd", ZIF_DIR },
    { "ze", ZIF_DIR_READ },
    { "zr", ZIF_READ },
    { "zw", ZIF_WRITE },
};

typedef enum fsm_state {
    IN_CMD,
    IN_WHITE_1,
    IN_HEX,
    IN_WHITE_2,
    ACCEPT,
    REJECT
} fsm_state_t;


static int get_cmd(unsigned char * buf, cmd_t * cmd);



void parse_ascii(unsigned char * buf, parse_result_t * res)
{
    fsm_state_t state = IN_CMD;
    int buf_ptr = 0;
    cmd_t prelim_cmd = INVALID;

    unsigned char * hex_start;

    while((state != ACCEPT) && (state != REJECT))
    {
        switch(state)
        {
            case IN_CMD:
                if(buf[buf_ptr] < 'a' || buf[buf_ptr] > 'z')
                {
                    if(buf_ptr == 2 &&
                       buf[buf_ptr] == ' ' &&
                       get_cmd(buf, &res->cmd) == 0)
                    {
                        state = IN_WHITE_1;
                    }
                    else
                    {
                        state = REJECT;
                    }
                }
                break;

            case IN_WHITE_1:
                if(buf[buf_ptr] != ' ')
                {
                    if((buf[buf_ptr] >= '0' && buf[buf_ptr] <= '9') ||
                            (buf[buf_ptr] >= 'A' && buf[buf_ptr] <= 'F'))
                    {
                        hex_start = buf + buf_ptr;
                        state = IN_HEX;
                        continue; // We actually need this value, so don't inc ptr.
                    }
                    else
                    {
                        state = REJECT;
                    }
                }
                break;

            case IN_HEX:
                {
                    size_t hex_len = (buf + buf_ptr) - hex_start;

                    if((buf[buf_ptr] < '0' ||
                        (buf[buf_ptr] > '9' && buf[buf_ptr] < 'A') ||
                        buf[buf_ptr] > 'F') &&
                       hex_len <= sizeof(res->arg))
                    {
                        if(buf[buf_ptr] == ' ' || buf[buf_ptr] == '\n' || buf[buf_ptr] == '\r')
                        {
                            ascii_to_hex(res->arg, hex_start, sizeof(res->arg), hex_len);
                            state = IN_WHITE_2;
                            continue; // Don't skip over possible \n!
                        }
                        else
                        {
                            state = REJECT;
                        }
                    }
                }
                break;

            case IN_WHITE_2:
                if(buf[buf_ptr] == ' ')
                {
                    // Continue
                }
                else if(buf[buf_ptr] == '\n' || buf[buf_ptr] == '\r')
                {
                    state = ACCEPT;
                }
                else
                {
                    state = REJECT;
                }
                break;
        }


        // Nuclear fail conditions
        if(buf_ptr >= 63)
        {
            state = REJECT;
        }

        buf_ptr++;
    }


    if(state == REJECT)
    {
        res->cmd = INVALID;
    }
}


static int get_cmd(unsigned char * buf, cmd_t * cmd)
{
    int cmd_size = sizeof(valid_cmds)/sizeof(cmd_map_t);
    int i;

    for(i = 0; i < cmd_size; i++)
    {
        if((valid_cmds[i].name[0] == buf[0]) &&
                (valid_cmds[i].name[1] == buf[1]))
        {
            (* cmd) = valid_cmds[i].cmd;
            return 0;
        }
    }

    return -1;
}



void ascii_to_hex(unsigned char * dst, unsigned char * src, size_t dst_len, size_t src_len)
{
    // Clear previous value so masking works.
    for(int i = 0; i < dst_len; i++)
    {
        dst[i] = 0x00;
    }

    for(int j = 0; j < src_len; j++)
    {
        // Store in little endian, but parse as if
        // the user wrote "left-to-right" format.

        unsigned char char_val = src[src_len - j - 1];
        unsigned char hex_val;

        if(char_val <= '9')
        {
            hex_val = char_val - '0';
        }
        else
        {
            hex_val = (char_val - 'A') + 10;
        }

        if(j & 0x01)
        {

            // Top nibble
            dst[j >> 1] |= (hex_val << 4);
        }
        else
        {
            // Bottom nibble.
            dst[j >> 1] |= (hex_val);
        }
    }
}


void hex_to_ascii(unsigned char * dst, unsigned char * src, size_t dst_len, size_t src_len)
{
    // Clear previous value so masking works.
    for(int i = 0; i < dst_len; i++)
    {
        dst[i] = 0x00;
    }

    for(int j = 0; j < src_len; j++)
    {
        // Store in little endian, but parse as if
        // the user wrote "left-to-right" format.

        unsigned char hex_val = src[j];
        unsigned char nibble_lo, nibble_hi;
        unsigned char char_lo, char_hi;

        nibble_lo = hex_val & 0x0F;
        nibble_hi = (hex_val >> 4) & 0x0F; // Sign bit shifted right?

        if(nibble_lo <= 0x09)
        {
            char_lo = nibble_lo + '0';
        }
        else
        {
            char_lo = nibble_lo + ('A' - 10);
        }

        if(nibble_hi <= 0x09)
        {
            char_hi = nibble_hi + '0';
        }
        else
        {
            char_hi = nibble_hi + ('A' - 10);
        }

        {
            // Write result backwards- as if user entered value in ASCII.
            unsigned char base = dst_len - (j << 1) - 1;

            dst[base] |= char_lo;
            dst[base - 1] |= char_hi;
        }
    }
}
