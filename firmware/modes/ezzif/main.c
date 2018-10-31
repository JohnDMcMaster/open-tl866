//27C256

//#include "epromv.h"
#include "../../mode.h"
#include "../../comlib.h"

#include "ezzif.h"

static inline void print_banner(void)
{
    com_println("   | |");
    com_println(" ==[+]==  open-tl866 Programmer Mode (EZZIF)");
    com_println("   | |    TASTY SNACK EDITION.");
}

static inline void print_help(void)
{
    com_println("Commands:");
    com_println("  0: digital I/O test");
    com_println("  1: VPP sweep test");
    com_println("  2: VDD sweep test");
    com_println("  3: multiple voltage rail test");
    com_println("  4: no ground test");
}

static void prompt_enter(void) {
    com_println("Press enter to continue");
    com_readline();
}

static void prompt_msg(const char *msg) {
    //ezzif_print_debug();
    com_println(msg);
    com_readline();
}

static inline void eval_command(unsigned char * cmd)
{
    unsigned char * cmd_t = strtok(cmd, " ");

    ezzif_reset();
    switch (cmd_t[0]) {
        case '0':
        {
            com_println("Digital test");
            com_println("1: 1, 2: 0, 3: 1, 4: 0, 5: Z, 6: Z");
            ezzif_gnd(20);
            ezzif_io_d40(1, 1, 1);
            ezzif_io_d40(2, 1, 0);
            ezzif_io_d40(3, 1, 1);
            ezzif_io_d40(4, 1, 0);
            ezzif_io_d40(5, 0, 0);
            ezzif_io_d40(6, 0, 0);
            printf("Done, error: %i\r\n", ezzif_error());
            prompt_enter();
            break;
        }

        case '1':
        {
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
            break;
        }

        case '2':
        {
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
            break;
        }

        case '3':
        {
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
            ezzif_gnd(40);
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
            ezzif_gnd(40);
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
            ezzif_gnd(40);
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
            break;
        }

        case '4':
        {
            ezzif_vpp_d40(1, VPP_98);
            ezzif_vdd_d40(2, VDD_48);
            prompt_msg("10 Check 10V: 1 to 40 (VPP to GND)");
            break;
        }

        case 's': {
            ezzif_print_debug();
            break;
        }

        case '?':
        case 'h':
            print_help();
            break;
        case 'V':
            //print_version();
            break;
        default:
            printf("Error: Unknown command.");
    }
    ezzif_reset();
}

void mode_main(void) {
    ezzif_reset();
    
    // Wait for user interaction (press enter).
    com_readline();

    print_banner();
    print_help();
    enable_echo();

    unsigned char * cmd;

    while(1) {
        printf("\r\nCMD> ");
        cmd = com_readline();
        com_println("");
        eval_command(cmd);
    }    
}

