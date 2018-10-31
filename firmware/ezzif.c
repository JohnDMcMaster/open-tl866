#include "ezzif.h"
#include "comlib.h"

#include <string.h>
#include <stdio.h>

#define D40_MASK(n) (1 << (((n) - 1) % 8))
#define D40_OFF(n) (((n) - 1) / 8)

//Direction
zif_bits_t ezzif_zbd = {0};
//Output
zif_bits_t ezzif_zbo = {0};
zif_bits_t ezzif_zb_vpp = {0};
zif_bits_t ezzif_zb_vdd = {0};
zif_bits_t ezzif_zb_gnd = {0};

const_zif_bits_t ezzif_zero = {0, 0, 0, 0, 0};

static int has_error = 0;

//Verify no pin has two drivers
int is_vsafe(void) {
    for (unsigned i = 0; i < 5; ++i) {
        unsigned char checks[4];

        checks[0] = ezzif_zbd[i];
        checks[1] = ezzif_zb_vpp[i];
        checks[2] = ezzif_zb_vdd[i];
        checks[3] = ezzif_zb_gnd[i];

        for (unsigned j = 0; j < sizeof(checks); ++j) {
            for (unsigned k = 0; k < sizeof(checks); ++k) {
                if (j != k && (checks[i] & checks[j])) {
                    has_error = 1;
                    return 0;
                }
            }
        }

    }
    return 1;
}

void ezzif_reset(void) {
    //ezzif_zbd = {0};
    memset(ezzif_zbd, 0, sizeof(ezzif_zbd));
    dir_write(ezzif_zbd);

    memset(ezzif_zbo, 0, sizeof(ezzif_zbo));
    zif_write(ezzif_zbo);

    ezzif_reset_vpp();
    ezzif_reset_vdd();
    ezzif_reset_gnd();

    //Make sure?
    for (unsigned i = 0; i < 8; ++i) {
        write_latch(i, 0x00);
    }
}

void ezzif_reset_vpp(void) {
    memset(ezzif_zb_vpp, 0, sizeof(ezzif_zb_vpp));
    set_vpp(ezzif_zb_vpp);
    vpp_dis();
}

void ezzif_reset_vdd(void) {
    memset(ezzif_zb_vdd, 0, sizeof(ezzif_zb_vdd));
    set_vdd(ezzif_zb_vdd);
    vdd_dis();
}

void ezzif_reset_gnd(void) {
    memset(ezzif_zb_gnd, 0, sizeof(ezzif_zb_gnd));
    set_gnd(ezzif_zb_gnd);
}

int ezzif_error(void) {
    int ret = has_error;
    has_error = 0;
    return ret;
}

void ezzif_print_debug(void) {
    port_bits_t p_bits;

    printf("ezzif state\r\n");
    printf("  ezzif error: %d\r\n", has_error);
    printf("  comlib drops: %u\r\n", comblib_drops);
    printf("\r\n");

    printf("  ezzif dir %02X %02X %02X %02X %02X\r\n",
            ezzif_zbd[0], ezzif_zbd[1], ezzif_zbd[2], ezzif_zbd[3], ezzif_zbd[4]);
    printf("  ezzif out %02X %02X %02X %02X %02X\r\n",
            ezzif_zbo[0], ezzif_zbo[1], ezzif_zbo[2], ezzif_zbo[3], ezzif_zbo[4]);
    printf("  ezzif VPP %02X %02X %02X %02X %02X\r\n",
            ezzif_zb_vpp[0], ezzif_zb_vpp[1], ezzif_zb_vpp[2], ezzif_zb_vpp[3], ezzif_zb_vpp[4]);
    printf("  ezzif VDD %02X %02X %02X %02X %02X\r\n",
            ezzif_zb_vdd[0], ezzif_zb_vdd[1], ezzif_zb_vdd[2], ezzif_zb_vdd[3], ezzif_zb_vdd[4]);
    printf("  ezzif GND %02X %02X %02X %02X %02X\r\n",
            ezzif_zb_gnd[0], ezzif_zb_gnd[1], ezzif_zb_gnd[2], ezzif_zb_gnd[3], ezzif_zb_gnd[4]);

    printf("\r\n");

    port_read_all(p_bits);
    printf("  MCU tristate A:%02X B:%02X C:%02X D:%02X E:%02X F:%02X G:%02X H:%02X J:%02X\r\n",
            p_bits[0], p_bits[1], p_bits[2], p_bits[3], p_bits[4],
            p_bits[5], p_bits[6], p_bits[7], p_bits[8]);

    dir_read_all(p_bits);
    printf("  MCU read A:%02X B:%02X C:%02X D:%02X E:%02X F:%02X G:%02X H:%02X J:%02X\r\n",
            p_bits[0], p_bits[1], p_bits[2], p_bits[3], p_bits[4],
            p_bits[5], p_bits[6], p_bits[7], p_bits[8]);

    printf("  Latch cache 0:%02X 1:%02X 2:%02X 3:%02X 4:%02X 5:%02X 6:%02X 7:%02X\r\n",
            latch_cache[0], latch_cache[1], latch_cache[2], latch_cache[3],
            latch_cache[4], latch_cache[5], latch_cache[6], latch_cache[7]);

    printf("  nVPP_OE: %d, nVDD_OE: %d\r\n", vpp_state(), vdd_state());
    printf("  OEn: %02X\r\n", OEn_state());
}


/****************************************************************************
DIP40
****************************************************************************/

int ezzif_valid_d40(int n) {
    return n >= 1 && n <= 40;
}

int ezzif_assert_d40(int n) {
    if (ezzif_valid_d40(n)) {
        return 1;
    } else {
        has_error = 1;
        return 0;
    }
}

void ezzif_toggle_d40(int n) {
    int ni = n - 1;
    int off = ni / 8;
    int mask = 1 << (ni % 8);

    if (!ezzif_assert_d40(n)) {
        return;
    }

    ezzif_zbo[off] ^= mask;
    zif_write(ezzif_zbo);
}

void ezzif_w_d40(int n, int val) {
    int ni = n - 1;
    int off = ni / 8;
    int mask = 1 << (ni % 8);
    int delta;

    if (!ezzif_assert_d40(n)) {
        return;
    }

    delta = (ezzif_zbo[off] & mask) ^ (val ? mask : 0);
    if (delta) {
        ezzif_zbo[off] = ezzif_zbo[off] ^ delta;
        zif_write(ezzif_zbo);
    }
}

void ezzif_dir_d40(int n, int isout) {
    int ni = n - 1;
    int off = ni / 8;
    int mask = 1 << (ni % 8);
    int delta;

    if (!ezzif_assert_d40(n)) {
        return;
    }

    delta = (ezzif_zbd[off] & mask) ^ (isout ? mask : 0);
    if (delta) {
        ezzif_zbd[off] = ezzif_zbd[off] ^ delta;
        if (!is_vsafe()) {
            return;
        }
        dir_write(ezzif_zbd);
    }
}

void ezzif_io_d40(int n, int isout, int val) {
    ezzif_dir_d40(n, isout);
    if (isout) {
        ezzif_w_d40(n, val);
    }
}

void ezzif_o_d40(int n, int val) {
    ezzif_io_d40(n, 1, val);
}

void ezzif_i_d40(int n) {
    ezzif_io_d40(n, 0, 0);
}

int ezzif_r_d40(int n) {
    zif_bits_t zb;
    int ni = n - 1;
    int off = ni / 8;
    int mask = 1 << (ni % 8);

    zif_read(zb);
    return zb[off] & mask ? 1 : 0;
}

void zif_bit_d40(zif_bits_t zb, int n) {
    int ni = n - 1;

    zb[ni / 8] |= 1 << (ni % 8);
}

void ezzif_vdd_d40(int n, int voltset) {
    vdd_val(voltset);

    zif_bit_d40(ezzif_zb_vdd, n);
    if (!is_vsafe()) {
        return;
    }
    set_vdd(ezzif_zb_vdd);
    vdd_en();
}

void ezzif_vpp_d40(int n, int voltset) {
    vpp_val(voltset);

    zif_bit_d40(ezzif_zb_vpp, n);
    if (!is_vsafe()) {
        return;
    }
    set_vpp(ezzif_zb_vpp);
    vpp_en();
}

void ezzif_gnd_d40(int n) {
    zif_bit_d40(ezzif_zb_gnd, n);
    if (!is_vsafe()) {
        return;
    }
    set_gnd(ezzif_zb_gnd);
}


/****************************************************************************
Generic DIP
****************************************************************************/

void ezzif_bus_dir(const char *ns, int len, int isout) {
    for (unsigned i = 0; i < len; ++i) {
        ezzif_dir(ns[i], isout);
    }
}

void ezzif_bus_w(const char *ns, int len, int val) {
    for (unsigned i = 0; i < len; ++i) {
        ezzif_w_d40(ezzif_dipto40(ns[i]), (val & (1 << i)) != 0);
    }
}

int ezzif_bus_r(const char *ns, int len) {
    int ret = 0;

    for (unsigned i = 0; i < len; ++i) {
        if (ezzif_r(ns[i])) {
            ret |= 1 << i;
        }
    }
    return ret;
}

