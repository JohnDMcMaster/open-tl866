#include <string.h>

#include "parse.h"

typedef enum fsm_state {
    IN_CMD,
    IN_WHITE_1,
    IN_HEX_8,
    IN_HEX_5,
    IN_WHITE_2,
    ACCEPT,
    REJECT
} fsm_state_t;

void parse_ascii(unsigned char * buf, parse_result_t * res)
{
    fsm_state_t state = IN_CMD;
    int buf_ptr = 0;
    
    while((state != ACCEPT) && (state != REJECT))
    {
        switch(state)
        {
            case IN_CMD:    
                break;
            
        }
        

        // Nuclear fail conditions
        if(buf_ptr >= 63)
        {
            state = REJECT;
            res->cmd = INVALID;
        }
        
        buf_ptr++;
    }
}

// Return: number of chars parsed. Must parse exactly "expected" chars before
// whitespace to succeed. limit is buffer len to prevent UB.
int get_cmd(unsigned char * buf, int expected, int limit)
{
    int i = 0;

    while(i < limit)
    {
        if(buf[i] == ' ' || buf[i] == '\t')
        {
            return (i == expected) ? i : 0;
        }
        
        i++;
    }
    
    return -2;
}



int ascii_to_hex(unsigned char * dst, unsigned char * src, int limit)
{
    int i;

    for(i = 0; i < limit; i++)
    {
        if(src[i] < '0' || (src[i] > '9' && src[i] < 'A') || src[i] > 'F')
        {
            return -1;
        }
        else
        {
            dst[i] = src[i] - '0';
        }
    }
    
    return 0;
}
