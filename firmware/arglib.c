#include "comlib.h"
#include "io.h"

int last_i = 0;
int last_bit = 0;
zif_bits_t last_zif;

int arg_i(void)
{
    const char *buff = strtok(NULL, " ");

    if (!buff) {
        printf("ERROR: missing argument\r\n");
        return 0;
    }

    last_i = atoi(buff);
    return 1;
}

int arg_bit(void)
{
    if (!arg_i()) {
        return 0;
    }
    last_bit = last_i ? 1 : 0;
    return 1;
}

int hex_c2i(char c)
{
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return -1;
    }
}

int arg_zif(void)
{
    const char *buff = strtok(NULL, " ");
    if (!buff) {
        printf("ERROR: missing argument\r\n");
        return 0;
    }

    if (strlen(buff) != 10) {
        printf("ERROR: expecting 10 hex digits\r\n");
        return 0;
    }
    for (int i = 0; i < 5; ++i) {
        // No error checking right now
        // Bad nibbles become 0
        int hi = hex_c2i(buff[i << 1]);
        int lo = hex_c2i(buff[(i << 1) + 1]);
        if (hi < 0 || lo < 0) {
            printf("ERROR: invalid hex digit\r\n");
            return 0;
        }
        last_zif[4 - i] = (hi << 4) | lo;
    }
    return 1;
}
