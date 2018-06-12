#include "at89.h"

inline void print_banner(void)
{
    com_println("   | |");
    com_println(" ==[+]==  open-tl866 Programmer Mode (AT89)");
    com_println("   | |");
}
inline void print_help(void)
{
    com_println("\r\nCommands:\r\n  r\tRead from target");
    com_println("  w\tWrite to target\r\n  e\tErase target");
    com_println("  h\tPrint help\r\n  v\tPrint version(s)\r\n");
}

inline void print_version()
{
    // All these should be defined in some config header files. TODO
    com_println("Programmer Mode - AT89 version: 0.0.1");
    com_println("open-tl866 lib version: UNIMPLEMENTED");
    com_println("");
}

void xfer_byte()
{
    // Setup pin direction
    // dir_write();
}


void setup(unsigned char * cmd)
{
    com_println("\r\nUnimplemented.");
}

// TODO
/*void or_pin(zif_bits_t &vdd, int pin_i)
{
    unsigned int byte = pin_i / 8;
    unsigned int bit  = pin_i % 8;
    vdd[byte] |= (char)1 >> bit;
}*/

void read_byte(unsigned char * cmd)
{
    /* 
     * AT89C51 Read Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4 or Vdd_9            // (high)
     * PSEN     <-      29          RD7                     // (low)
     * ALE      <-      30          RG0 or Vdd_30           // Pulsed prog.
     * EA       <-      31          RJ0                     // (high)
     * VCC      <-      40          Vdd_40
     * P0.{0-7) ->      39-32       RB{6,5,4,3,2}, RJ{3,2,1}      // PGM Data
     * P1.{0-7} <-      1-8         RC{5,4,3,2}, RJ{7,6}, RC{6,7} // Addr
     * P2.{0-3} <-      21-24       RE{4,5,6,1}             // Addr (contd.)
     * P2.6     <-      27          RD5                     // ctrl (low)
     * P2.7     <-      28          RD6                     // ctrl (low)
     * P3.4     ->      14          RD1                     // Busy
     * P3.6     <-      16          RG1                     // ctrl (high)
     * P3.7     <-      17          RE0                     // ctrl (high)
     * >
     * 
     * 
     */
    
    // TODO: Actually use this
    printf("\r\nAddress of requested byte? ");
    int addr = atoi(com_readline());
    printf("\r\naddr = %i\r\n", addr);

    zif_bits_t zbits_null = {   0b00000000,
                                0b00000000,
                                0b00000000,
                                0b00000000,
                                0b00000000 };
    
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0,
                        0b10000000,   // p0.7 (32)
                        0b01111111 }; // p0.{6-0} (33-39)  
    
    zif_bits_t vdd = {  0, 0, 0, 0,
                        0b10000000 }; // VDD (40)
    
    zif_bits_t gnd = {  0, 0,
                        0b00001000,   // GND (20)
                        0, 0 };

    printf("Setting pin direction.\r\n");
    dir_write(dir);
    
    printf("Setting pins.\r\n");  
    set_vdd(vdd);
    set_gnd(gnd);
    
    printf("Setting voltages.\r\n");
    vdd_val(5); // 5.0 v - 5.2 v
    
    printf("Press ENTER when ready.");
    
    com_readline();
    
    zif_bits_t read_setup_clk0 =     {   0b00000000,
                                    0b10000001, // 3.6 ctrl (16), RST (9)
                                    0b00000001, // 3.7 ctrl (17) 
                                    0b01100000, // EA (31), ALE (30)
                                    0b00000000 };
    
    zif_bits_t read_setup_clk1 =     {   0b00000000,
                                    0b10000001, // 3.6 ctrl (16), RST (9)
                                    0b00000101, // XTAL1 (19), 3.7 ctrl (17)
                                    0b01100000, // EA (31), ALE (30)
                                    0b00000000 };
    
    for(unsigned int i = 65525; i > 0; i--) {
        zif_write(read_setup_clk0); // Better flip XTAL1 directly than to
                                    // call zif_write every time.
        
        zif_write(read_setup_clk1);
    }
    
    printf("Done.\r\n");
    
    zif_write(zbits_null);
}

void write(unsigned char * cmd)
{
    switch (cmd[1]) {
        
    }
}

void erase(unsigned char * cmd)
{
    switch (cmd[1]) {
        
    }
}

void verify(unsigned char * cmd)
{
    switch (cmd[1]) {
        
    }
}

inline void eval_command(unsigned char * cmd)
{
    switch (cmd[0]) {
        case 's':
            setup(cmd);
            break;
            
        case 'r':
            read_byte(cmd);
            break;
            
        case 'w':
            write(cmd);
            break;
            
        case 'e':
            erase(cmd);
            break;

        case 'v':
            verify(cmd);
            break;
            
        case '?':
        case 'h':
            print_help();
            break;
        case 'V':
            print_version();
            break;
        default:
            printf("\r\nError: Unknown command.\r\n");
    }
}

int programmer_at89(void) {
    
    // Wait for user interaction (press enter).
    com_readline();
    
    print_banner();
    print_help();
    enable_echo();
    
    unsigned char * cmd;
    
    while(1) {
        printf("CMD> ");
        cmd = com_readline();
        eval_command(cmd);
    }
    
}