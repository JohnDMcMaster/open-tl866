//27C256

//#include "epromv.h"
#include "../../mode.h"
#include "../../comlib.h"
#include "../../stock_compat.h"

#include "ezzif.h"

int main_debug = 0;

static inline void print_help(void)
{
    com_println("open-tl866 (ezzif)");
    com_println("0      digital I/O test");
    com_println("1      bus I/O test");
    com_println("2      VPP sweep test");
    com_println("3      VDD sweep test");
    com_println("4      multiple voltage rail test");
    com_println("5      no ground test");
    com_println("d      debug status");
    com_println("b      reset to bootloader");
}

static void prompt_msg(const char *msg) {
    if (main_debug) {
        ezzif_print_debug();
    }
    com_println(msg);
    com_readline();
}

static void prompt_enter(void) {
    prompt_msg("Press enter to continue");
}

static void test_io(void) {
    com_println("Digital I/O test");
    com_println("1: 1, 2: 0, 3: 1, 4: 0, 5: Z, 6: Z");
    ezzif_gnd_d40(40);
    ezzif_io_d40(1, 0, 1);
    ezzif_io_d40(2, 0, 0);
    ezzif_io_d40(3, 0, 1);
    ezzif_io_d40(4, 0, 0);
    ezzif_io_d40(5, 1, 1);
    ezzif_io_d40(6, 1, 0);
    printf("Done, error: %i\r\n", ezzif_error());
    prompt_enter();
}

static void test_bus(void) {
    static const char BUS[] = {1, 2};

    com_println("Bus test");
    ezzif_gnd_d40(40);
    ezzif_bus_dir(BUS, sizeof(BUS), 0);
    for (unsigned i = 0; i < 4; ++i) {
        ezzif_bus_w(BUS, sizeof(BUS), i);
        printf("Set bus: %u\r\n", i);
        prompt_enter();
    }

    printf("Done, error: %i\r\n", ezzif_error());
    prompt_enter();
}

static void test_vpp(void) {
    static const char VPPS[] = {VPP_98, VPP_126, VPP_140, VPP_166, VPP_144, VPP_171, VPP_185, VPP_212};
    com_println("Sweeping VPP pin 1, gnd 40");
    com_println("Reference: 9.83, 12.57, 14.00, 16.68, 14.46, 17.17, 18.56, 21.2");
    ezzif_gnd_d40(40);
    for (unsigned i = 0; i < sizeof(VPPS); ++i) {
        ezzif_vpp_d40(1, VPPS[i]);
        printf("Set enum %u\r\n", VPPS[i]);
        prompt_enter();
    }
    printf("Done, error: %i\r\n", ezzif_error());
}

static void test_vdd(void) {
    static const char VDDS[] = {VDD_30, VDD_35, VDD_46, VDD_51, VDD_43, VDD_48, VDD_60, VDD_65};
    com_println("Sweeping VDD pin 1, gnd 40");
    com_println("Reference: 2.99, 3.50, 4.64, 5.15, 4.36, 4.86, 6.01, 6.52");
    ezzif_gnd_d40(40);
    for (unsigned i = 0; i < sizeof(VDDS); ++i) {
        ezzif_vdd_d40(1, VDDS[i]);
        printf("Set enum %u\r\n", VDDS[i]);
        prompt_enter();
    }
    printf("Done, error: %i\r\n", ezzif_error());
}

void test_rails(void) {
    com_println("Voltage rail test");
    com_println("1: VPP 10V");
    com_println("2: VDD 5V");
    com_println("40: GND");

    com_println("");


    /*
    Test rails operate normally
    */

    com_println("");
    com_println("Testing normal operation");
    ezzif_reset();
    ezzif_vpp_d40(1, VPP_98);
    ezzif_gnd_d40(40);
    prompt_msg("10 Check 10V: 1 to 40 (VPP to GND)");

    ezzif_vdd_d40(2, VDD_48);
    prompt_msg("11 Check 5V: 2 to 40 (VDD to GND)");
    prompt_msg("12 Check 5V: 1 to 2 (VPP to VDD)");


    /*
    Turn off rails
    Verify no voltage
    */

    com_println("");
    com_println("Testing rail shut off");
    ezzif_reset_vpp();
    ezzif_gnd_d40(40);
    prompt_msg("20 Check 0V: 1 to 40 (VPP off)");
    prompt_msg("21 Check 5V: 2 to 40 (VDD to GND)");

    ezzif_reset_vdd();
    prompt_msg("22 Check 0V: 2 to 40 (VDD off)");

    
    /*
    Turn off ground
    Verify no voltage
    */

    /*
    com_println("");
    com_println("Testing ground shut off");
    com_println("WARNING: test faulty?");
    ezzif_reset();
    ezzif_gnd_d40(40);
    ezzif_vpp_d40(1, VPP_98);
    ezzif_vdd_d40(2, VDD_48);
    prompt_msg("30 Check 10V: 1 to 40 (VPP to GND)");
    prompt_msg("31 Check 5V: 2 to 40 (VDD to GND)");

    ezzif_reset_gnd();
    prompt_msg("33 Check 0V: 1 to 40 (VPP GND off)");
    prompt_msg("34 Check 0V: 2 to 40 (VDD GND off)");
    */

    com_println("");
    printf("Done, error: %i\r\n", ezzif_error());
}

static void test_gnd(void) {
    ezzif_vpp_d40(1, VPP_98);
    ezzif_vdd_d40(2, VDD_48);
    prompt_msg("10 Check 10V: 1 to 40 (VPP to GND)");
}

static inline void eval_command(unsigned char * cmd)
{
    unsigned char * cmd_t = strtok(cmd, " ");

    if (cmd_t == NULL) {
        return;
    }

    ezzif_reset();
    switch (cmd_t[0]) {
    case '0':
        test_io();
        break;

    case '1':
        test_bus();
        break;

    case '2':
        test_vpp();
        break;

    case '3':
        test_vdd();
        break;

    case '4':
        test_rails();
        break;

    case '5':
        test_gnd();
        break;

    case 'd': {
        main_debug = 1;
        ezzif_print_debug();
        break;
    }

    case 'D': {
        main_debug = 0;
        break;
    }

    case '?':
    case 'h':
        print_help();
        break;

    case 'b':
        stock_reset_to_bootloader();
        break;

    default:
        printf("Error: Unknown command 0x%02X (%c)\r\n", cmd_t[0], cmd_t[0]);
        break;
    }
    ezzif_reset();
}

void mode_main(void) {
    ezzif_reset();

    while(1) {
        eval_command(com_cmd_prompt());
    }
}

