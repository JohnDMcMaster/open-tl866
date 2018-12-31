//#define DEBUG(x) x
#define DEBUG(x) do {} while(0)

#include <xc.h>

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
zif_bits_t ezzif_zb_read = {0};

const_zif_bits_t ezzif_zero = {0, 0, 0, 0, 0};

static int has_error = 0;

//Verify no pin has two drivers
int is_vsafe(void) {
    for (unsigned i = 0; i < 5; ++i) {
        unsigned char checks[4];

        //tristate => don't drive
        checks[0] = ezzif_zbd[i] ^ 0xFF;
        checks[1] = ezzif_zb_vpp[i];
        checks[2] = ezzif_zb_vdd[i];
        checks[3] = ezzif_zb_gnd[i];

        for (unsigned j = 0; j < sizeof(checks); ++j) {
            for (unsigned k = 0; k < sizeof(checks); ++k) {
                if (j != k && (checks[j] & checks[k])) {
                    printf("ERROR: is_vsafe(), i %d, j %d => %02X, k %d => %02X\r\n",
                        i, j, checks[j], k, checks[k]);
                    has_error = 1;
                    return 0;
                }
            }
        }

    }
    return 1;
}

void ezzif_reset(void) {
    has_error = 0;

    //Disable pullup/pulldown
    pupd(1, 0);

    //ezzif_zbd = {0};
    //tristate all pins
    memset(ezzif_zbd, 0xFF, sizeof(ezzif_zbd));
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

void ezzif_read(void) {
    zif_read(ezzif_zb_read);
}

void ezzif_print_debug(void) {
    port_bits_t p_bits;

    printf("ezzif state\r\n");
    printf("  ezzif error: %d\r\n", has_error);
    printf("  comlib drops: %u\r\n", comblib_drops);

    printf("\r\n");

    print_zif_bits("  ezzif dir", ezzif_zbd);
    print_zif_bits("  ezzif out", ezzif_zbo);
    ezzif_read();
    print_zif_bits("  ezzif read", ezzif_zb_read);
    print_zif_bits("  ezzif VPP", ezzif_zb_vpp);
    print_zif_bits("  ezzif VDD", ezzif_zb_vdd);
    print_zif_bits("  ezzif GND", ezzif_zb_gnd);

    printf("\r\n");

    port_read_all(p_bits);
    print_port_bits("  MCU read", p_bits);
    dir_read_all(p_bits);
    print_port_bits("  MCU tristate", p_bits);
    print_latch_bits("  Latch cache", latch_cache);
    printf("  VPP_OEn: %d, VDD_OEn: %d\r\n", vpp_state(), vdd_state());
    printf("  PUPD tris: %u, port: %u\r\n", PUPD_TRIS, PUPD_PORT);
    DEBUG(printf("  Latch OEn: %02X\r\n", OEn_state()));
    DEBUG(printf("  SR CLK:%u DAT:%u\r\n", SR_CLK, SR_DAT));

    printf("  VPP ref port %u %u %u\r\n", VID_10, VID_11, VID_12);
    printf("  VPP ref tris %u %u %u\r\n", VID_10_TRIS, VID_11_TRIS, VID_12_TRIS);
    printf("  VDD ref port %u %u %u\r\n", VID_00, VID_01, VID_01);
    printf("  VDD ref tris %u %u %u\r\n", VID_00_TRIS, VID_01_TRIS, VID_01_TRIS);
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
        printf("ERROR: ezzif_assert_d40()\r\n");
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

void ezzif_dir_d40(int n, int tristate) {
    int ni = n - 1;
    int off = ni / 8;
    int mask = 1 << (ni % 8);
    int delta;

    if (!ezzif_assert_d40(n)) {
        return;
    }

    delta = (ezzif_zbd[off] & mask) ^ (tristate ? mask : 0);
    if (delta) {
        ezzif_zbd[off] = ezzif_zbd[off] ^ delta;
        if (!is_vsafe()) {
            return;
        }
        dir_write(ezzif_zbd);
    }
}

void ezzif_io_d40(int n, int tristate, int val) {
    ezzif_dir_d40(n, tristate);
    if (!tristate) {
        ezzif_w_d40(n, val);
    }
}

void ezzif_o_d40(int n, int val) {
    ezzif_io_d40(n, 0, val);
}

void ezzif_i_d40(int n) {
    ezzif_io_d40(n, 1, 0);
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
    //VDD required for VPP?
    vdd_en();
    vpp_en();
}

void ezzif_gnd_d40(int n) {
    zif_bit_d40(ezzif_zb_gnd, n);
    if (!is_vsafe()) {
        return;
    }
    set_gnd(ezzif_zb_gnd);
}


void ezzif_bus_dir_d40(const char *ns, unsigned len, int tristate) {
    for (unsigned i = 0; i < len; ++i) {
        ezzif_dir_d40(_ezzif_dipto40(ns[i]), tristate);
    }
}

void ezzif_bus_w_d40(const char *ns, unsigned len, uint16_t val) {
    for (unsigned i = 0; i < len; ++i) {
        ezzif_w_d40(_ezzif_dipto40(ns[i]), (val & (1 << i)) != 0);
    }
}

uint16_t ezzif_bus_r_d40(const char *ns, unsigned len) {
    int ret = 0;

    for (unsigned i = 0; i < len; ++i) {
        if (ezzif_r_d40(_ezzif_dipto40(ns[i]))) {
            ret |= 1 << i;
        }
    }
    return ret;
}


/****************************************************************************
Generic DIP
****************************************************************************/

int _ezzif_pins = 40;
int _ezzif_pins_div_2 = 20;

void ezzif_set_dip_pins(int n) {
    if (ezzif_valid_d40(n) != 1) {
        printf("ERROR: ezzif_set_dip_pins not in 1-40 inclusive.");
        return;
    }
    if ((n % 2) == 1) {
        printf("ERROR: ezzif_set_dip_pins must be even.");
        return;
    }
    _ezzif_pins = n;
    _ezzif_pins_div_2 = n >> 1;
}

void ezzif_bus_dir(const char *ns, unsigned len, int tristate) {
    for (unsigned i = 0; i < len; ++i) {
        ezzif_dir(ns[i], tristate);
    }
}

void ezzif_bus_w(const char *ns, unsigned len, uint16_t val) {
    for (unsigned i = 0; i < len; ++i) {
        ezzif_w(ns[i], (val & (1 << i)) != 0);
    }
}

uint16_t ezzif_bus_r(const char *ns, unsigned len) {
    int ret = 0;

    for (unsigned i = 0; i < len; ++i) {
        if (ezzif_r(ns[i])) {
            ret |= 1 << i;
        }
    }
    return ret;
}

