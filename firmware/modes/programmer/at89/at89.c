#include "at89.h"

inline void print_banner(void)
{
    com_println("   | |");
    com_println(" ==[+]==  open-tl866 Programmer Mode (AT89)");
    com_println("   | |");
}
inline void print_help(void)
{
    com_println("\r\nCommands:\r\n  r <ADDR>\tRead from target");
    com_println("  w <ADDR>\tWrite to target\r\n  e\tErase target");
    com_println("  h\tPrint help\r\n  v\tPrint version(s)\r\n");
}

inline void print_version()
{
    // All these should be defined in some config header files. TODO
    com_println("Programmer Mode - AT89 version: 0.0.1");
    com_println("open-tl866 lib version: UNIMPLEMENTED");
    com_println("");
}

void read_byte(unsigned char * cmd)
{
    /* 
     * AT89C51 Read Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                     // (high)
     * PSEN     <-      29          RD7                     // (low)
     * ALE      <-      30          RG0                     // (high)
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
    
    unsigned int addr = atoi(strtok(NULL, " "));
    
    printf("\r\n%u", addr);

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

    // Set pin direction
    dir_write(dir);
    
    // Set Vdd / GND pinout  
    set_vdd(vdd);
    set_gnd(gnd);
    
    // Setting voltages
    vdd_val(5); // 5.0 v - 5.2 v
    
    zif_bits_t input_byte = {0,0,0,0,0};
    
    zif_bits_t read_clk0 =     {        0b00000000,
                                        0b10000001, // 3.6 ctrl (16), RST (9)
                                        0b00000001, // 3.7 ctrl (17) 
                                        0b01100000, // EA (31), ALE (30)
                                        0b00000000 };

    zif_bits_t read_clk1;
    memcpy(read_clk1, read_clk0, sizeof read_clk1);
    read_clk1[2] = read_clk0[2] | 0b00000100;
    
    zif_bits_t read_setup_clk1_d =     {  0b00000000,
                                        0b10000001, // 3.6 ctrl (16), RST (9)
                                        0b00000101, // XTAL1 (19), 3.7 ctrl (17)
                                        0b01100000, // EA (31), ALE (30)
                                        0b00000000 };
    
    // Mask the addrs in
    read_clk0[0] = addr & 255;
    read_clk0[2] = (addr >> 8) << 4 | read_clk0[2];
    
    read_clk1[0] = addr & 255;
    read_clk1[2] = (addr >> 8) << 4 | read_clk1[2];
    
    for(unsigned char i = 0; i <= 48; i++) {
        zif_write(read_clk0); // Maybe better flip XTAL1 directly than to
                                    // call zif_write every time. TODO
        zif_write(read_clk1);
    }
    
    zif_read(input_byte);
    
    zif_write(zbits_null);
    
    unsigned char byte;
    {
        static unsigned char lookup[16] = {
                            0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
                            0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };
    
        // Filter the zif_bits response into a char byte with P0 bits
        byte = (input_byte[4] << 1) | !! (input_byte[3] & (1 << 7));
        
        // Invert bit-endianness
        byte = (lookup[byte&0b1111] << 4) | lookup[byte>>4];
    }

    printf(" %02X\r\n", byte );
}

void write_byte(unsigned char * cmd)
{
    /* 
     * AT89C51 write Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                     // (high)
     * PSEN     <-      29          RD7                     // (low)
     * PROG      <-      30          RG0                     // Pulsed prog.
     * VPP       <-      31          VPP_31                  // 12v
     * VCC      <-      40          Vdd_40
     * P0.{0-7) <-      39-32       RB{6,5,4,3,2}, RJ{3,2,1}      // PGM Data
     * P1.{0-7} <-      1-8         RC{5,4,3,2}, RJ{7,6}, RC{6,7} // Addr
     * P2.{0-3} <-      21-24       RE{4,5,6,1}             // Addr (contd.)
     * P2.6     <-      27          RD5                     // ctrl (low)
     * P2.7     <-      28          RD6                     // ctrl (high)
     * P3.4     ->      14          RD1                     // Busy
     * P3.6     <-      16          RG1                     // ctrl (high)
     * P3.7     <-      17          RE0                     // ctrl (high)
     * >
     * 
     * 
     */
    
    unsigned int addr  = atoi(strtok(NULL, " "));
    unsigned char byte = atoi(strtok(NULL, " "));
    
    printf("\r\nWriting byte %x at address %x\r\n", byte, addr);

    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0, 0, 0 };
    
    zif_bits_t vdd = {  0, 0, 0, 0,
                        0b10000000 }; // VDD (40)

    zif_bits_t vpp = {  0, 0, 0,
                        0b01000000,   // VPP (31)
                        0 };
    
    zif_bits_t gnd = {  0, 0,
                        0b00001000,   // GND (20)
                        0, 0 };

    printf("Setting pin direction.\r\n");
    dir_write(dir);
    
    printf("Setting pins.\r\n");  
    set_vdd(vdd);
    set_vpp(vpp);
    set_gnd(gnd);
    
    printf("Setting voltages.\r\n");
    vdd_val(5); // 5.0 v - 5.2 v
    vpp_val(1); // 12.8 - 13.2
    
    zif_bits_t write_preclk =   { 0b00000000,
                                  0b10000001, // 3.6 ctrl (16), RST (9)
                                  0b00000101, // XTAL1 (19), 3.7 ctrl (17)
                                  0b01101000, // VPP (31), PROG (30), 2.7 (28)
                                  0b00000000 };

    zif_bits_t write_clk0 =     { 0b00000000,
                                  0b10000001, // 3.6 ctrl (16), RST (9)
                                  0b00000001, // 3.7 ctrl (17) 
                                  0b01001000, // VPP (31), 2.7 (28)
                                  0b00000000 };

    zif_bits_t write_clk1 =     { 0b00000000,
                                  0b10000001, // 3.6 ctrl (16), RST (9)
                                  0b00000101, // XTAL1 (19), 3.7 ctrl (17)
                                  0b01001000, // VPP (31), 2.7 (28)
                                  0b00000000 };
    
    // Mask address bits
    write_clk0[0] = addr & 255;
    write_clk0[2] = (addr >> 8) << 4 | write_clk0[2];
    
    write_clk1[0] = addr & 255;
    write_clk1[2] = (addr >> 8) << 4 | write_clk1[2];
    
    write_preclk[0] = addr & 255;
    write_preclk[2] = (addr >> 8) << 4 | write_preclk[2];

    // Mask data bits
    {
        static unsigned char lookup[16] = {
                            0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
                            0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };
        
        unsigned char mask_4 = byte & 127;
        unsigned char mask_sw_4 = (lookup[mask_4 & 0b1111] << 4) | lookup[mask_4 >> 4];
        
        write_clk0[3] = (byte & 128) | write_clk0[3];
        write_clk0[4] = (mask_sw_4 >> 1) | write_clk0[4];

        write_clk1[3] = (byte & 128) | write_clk1[3]; 
        write_clk1[4] = (mask_sw_4 >> 1) | write_clk1[4];
        
        write_preclk[3] = (byte & 128) | write_preclk[3]; 
        write_preclk[4] = (mask_sw_4 >> 1) | write_preclk[4];
    }

    vpp_en();
    
    zif_write(write_preclk);
    
    __delay_us(20);

    for(unsigned char i = 0; i <= 48; i++) {
        zif_write(write_clk0); // Better flip XTAL1 directly than to
                                    // call zif_write every time.
        
        zif_write(write_clk1);
    }
    
    vpp_dis();
    
    zif_write(zbits_null);
}

void erase(unsigned char * cmd)
{
    /* 
     * AT89C51 erase Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                     // (high)
     * PSEN     <-      29          RD7                     // (low)
     * ALE      <-      30          RG0                     // Pulsed erase
     * VPP       <-      31          VPP_31                  // 12v
     * VCC      <-      40          Vdd_40
     * P2.6     <-      27          RD5                     // ctrl (high)
     * P2.7     <-      28          RD6                     // ctrl (low)
     * P3.4     ->      14          RD1                     // Busy
     * P3.6     <-      16          RG1                     // ctrl (low)
     * P3.7     <-      17          RE0                     // ctrl (low)
     * >
     */
    
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0, 0, 0 };
    
    zif_bits_t vdd = {  0, 0, 0, 0,
                        0b10000000 }; // VDD (40)

    zif_bits_t vpp = {  0, 0, 0,
                        0b01000000,   // VPP (31)
                        0 };
    
    zif_bits_t gnd = {  0, 0,
                        0b00001000,   // GND (20)
                        0, 0 };

    // Setting pin direction
    dir_write(dir);
    
    // Setting pins
    set_vdd(vdd);
    set_vpp(vpp);
    set_gnd(gnd);
    
    // Setting voltages
    vdd_val(5); // 5.0 v - 5.2 v
    vpp_val(1); // 12.8 - 13.2
    
    zif_bits_t erase_preclk =     {     0b00000000,
                                        0b00000001, // RST (9)
                                        0b00000100, // XTAL1 (19)
                                        0b01100100, // VPP (31), PROG (30), 2.6 (27)
                                        0b00000000 };
    
    zif_bits_t erase_clk0 =     {       0b00000000,
                                        0b00000001, // RST (9)
                                        0b00000000,  
                                        0b01000100, // VPP (31), 2.6 (27)
                                        0b00000000 };

    zif_bits_t erase_clk1 =     {       0b00000000,
                                        0b00000001, // RST (9)
                                        0b00000100, // XTAL1 (19)
                                        0b01000100, // VPP (31), 2.6 (27)
                                        0b00000000 };
    
    vpp_en();
    
    zif_write(erase_preclk);
    
    __delay_us(20);
    
    for(unsigned char i = 0; i <= 48; i++) {
        zif_write(erase_clk0); // Better flip XTAL1 directly than to
                                    // call zif_write every time.
        
        zif_write(erase_clk1);
    }
    
    vpp_dis();
    
    zif_write(zbits_null);
    
    printf("\r\nErased.\r\n");
    
    
}

void verify(unsigned char * cmd)
{
    printf("\r\nUnimplemented.\r\n");
}

inline void eval_command(unsigned char * cmd)
{
    unsigned char * cmd_t = strtok(cmd, " ");
    switch (cmd_t[0]) {
        case 'r':
            read_byte(cmd_t);
            break;
            
        case 'w':
            write_byte(cmd_t);
            break;
            
        case 'e':
            erase(cmd_t);
            break;

        case 'v':
            verify(cmd_t);
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
