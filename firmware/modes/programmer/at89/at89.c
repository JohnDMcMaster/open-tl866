#include "at89.h"

inline void print_banner(void)
{
    com_println("   | |");
    com_println(" ==[+]==  open-tl866 Programmer Mode (AT89)");
    com_println("   | |");
}
inline void print_help(void)
{
    com_println("\r\nCommands:\r\n  r <ADDR> [RANGE]\tRead from target");
    com_println("  w <ADDR> <BYTE>\tWrite to target\r\n  e\t\t\tErase target");
    com_println("  h\t\t\tPrint help\r\n  v\t\t\tPrint version(s)\r\n");
}

inline void print_version()
{
    // All these should be defined in some config header files. TODO
    com_println("Programmer Mode - AT89 version: 0.0.1");
    com_println("open-tl866 lib version: UNIMPLEMENTED");
    com_println("");
}

// Neat trick taken from a stack overflow answer.
inline unsigned char invert_bit_endianness(unsigned char byte)
{
    static unsigned char lookup[16] = {
                            0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
                            0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };
    return (lookup[byte & 0b1111] << 4) | lookup[byte >> 4];
}

inline void mask_xtal1(zif_bits_t op_base)
{
    op_base[2] = op_base[2] | 4;
}

inline void mask_prog(zif_bits_t op_base)
{
    op_base[3] = op_base[3] | 32;
}

inline void mask_addr(zif_bits_t op_base, unsigned char addr)
{
    op_base[0] = addr & 255;
    op_base[2] = (addr >> 8) << 4 | op_base[2];
}

inline void mask_data(zif_bits_t op_base, unsigned char data)
{
    op_base[3] = (data & 128) | op_base[3];
    op_base[4] = (invert_bit_endianness(data & 127) >> 1) | op_base[4];
}

inline void mask_p2_7(zif_bits_t op_base)
{
    op_base[3] = op_base[3] | 8;
}

inline void mask_p3_6(zif_bits_t op_base)
{
    op_base[1] = op_base[1] | 128;
}

inline void mask_p3_7(zif_bits_t op_base)
{
    op_base[2] = op_base[2] | 1;
}

inline unsigned char zif_to_addr(zif_bits_t zif_state)
{
    // Filter the zif_bits response into a char byte with P0 bits
    unsigned char byte = (zif_state[4] << 1) | !! (zif_state[3] & (1 << 7));

    // Invert bit-endianness
    byte = invert_bit_endianness(byte);
}

inline void zif_clock_write(zif_bits_t op_template, zif_bits_t op_clk,
                            unsigned int cycles)
{
    for(unsigned char i = 0; i <= cycles; i++) {
        zif_write(op_template); // Maybe better flip XTAL1 directly than to
                                // call zif_write every time. TODO
        zif_write(op_clk);
    }
}

void read(unsigned int addr, unsigned int range)
{
    /* 
     * AT89C51 Read Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                     // (high)
     * PSEN     <-      29          RD7                     // (low)
     * PROG     <-      30          RG0                     // (high)
     * VPP      <-      31          RJ0                     // (high)
     * VCC      <-      40          Vdd_40
     * P0.{0-7) ->      39-32       RB{6,5,4,3,2}, RJ{3,2,1}      // PGM Data
     * P1.{0-7} <-      1-8         RC{5,4,3,2}, RJ{7,6}, RC{6,7} // Addr
     * P2.{0-3} <-      21-24       RE{4,5,6,1}             // Addr (contd.)
     * P2.6     <-      27          RD5                     // ctrl (low)
     * P2.7     <-      28          RD6                     // ctrl (low)
     * P3.4     ->      14          RD1                     // Busy
     * P3.6     <-      16          RG1                     // ctrl (high)
     * P3.7     <-      17          RE0                     // ctrl (high)
     */
    
    printf("\r\n%02X", addr);

    // Set pin direction
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0,
                        0b10000000,   // p0.7 (32)
                        0b01111111 }; // p0.{6-0} (33-39)  
    dir_write(dir);
    
    // Set Vdd / GND pinout  
    set_vdd(vdd);
    set_gnd(gnd);
    
    // Set voltages
    vdd_val(5); // 5.0 v - 5.2 v

    // Allocate an empty zifbits struct for reading pin state
    zif_bits_t input_byte    = { 0, 0, 0, 0, 0 };
    
    // Base pin setting for reading
    zif_bits_t read_base = { 0b00000000,
                             0b10000001,   // 3.6 ctrl (16), RST (9)
                             0b00000001,   // 3.7 ctrl (17) 
                             0b01100000,   // VPP (31), PROG (30)
                             0b00000000 };

    zif_bits_t read_clk;
    
    if (!range) { range = 1; } else {com_println("");}
    for (unsigned int byte_idx = 0; byte_idx < range; byte_idx++) {
        // Mask in the address bits to the appropriate pins
        mask_addr(read_base, addr + byte_idx);

        // Create a zif state with the clock pin turned on
        memcpy(read_clk, read_base, 5);
        mask_xtal1(read_clk);

        // Give the clock on/off states to zif_clock_write(..) and loop 48 cycles
        zif_clock_write(read_base, read_clk, 48);

        // Read the current pin state (to read in the requested byte)
        zif_read(input_byte);
    
        // We're done with the byte. Turn off all outputs.
        zif_write(zbits_null);

        // Parse and print interpreted byte
        printf(" %02X", zif_to_addr(input_byte) );
    }
    com_println("");
}

void write(unsigned int addr, unsigned char data)
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
     */

    // Set pin direction
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0, 0, 0 };
    dir_write(dir);
    
    // Set pins
    set_vdd(vdd);
    set_vpp(vpp);
    set_gnd(gnd);
    
    // Set voltages
    vdd_val(5); // 5.0 v - 5.2 v
    vpp_val(1); // 12.8 v - 13.2 v

    // Base pin setting for writing
    zif_bits_t write_base = { 0b00000000,
                              0b10000001,   // 3.6 ctrl (16), RST (9)
                              0b00000001,   // 3.7 ctrl (17) 
                              0b01001000,   // VPP (31), 2.7 (28)
                              0b00000000 };
    
    // Mask in our address and data outputs to the base pin configuration
    mask_addr(write_base, addr);
    mask_data(write_base, data);
    
    // Create a zif state to set before running the clock
    // PROG needs to be pulsed, can't be kept low the entire time.
    zif_bits_t write_preclk;
    memcpy(write_preclk, write_base, 5);
    mask_prog(write_preclk);

    // Create a zif state with the clock pin turned on
    zif_bits_t write_clk;
    memcpy(write_clk, write_base, 5);
    mask_xtal1(write_clk);
   
    // Enable VPP right before setting the ZIF state
    vpp_en();
    
    // Set PROG high before pulsing it low during programming
    zif_write(write_preclk);
    __delay_us(20); // 20us might be too generous. TODO

    zif_clock_write(write_base, write_clk, 48);

    // We're done. Disable VPP and reset the ZIF state.
    vpp_dis();
    zif_write(zbits_null);
    
    // The client / user is expected to verify this with a read command.
    printf("\r\nWrote byte %x at address %x\r\n", data, addr);
}

void erase()
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
     */
    
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0, 0, 0 };
    
    // Set pin direction
    dir_write(dir);
    
    // Set pins
    set_vdd(vdd);
    set_vpp(vpp);
    set_gnd(gnd);
    
    // Set voltages
    vdd_val(5); // 5.0 v - 5.2 v
    vpp_val(1); // 12.8 - 13.2
    
    // Base pin setting for erasing
    zif_bits_t erase_base =     {       0b00000000,
                                        0b00000001, // RST (9)
                                        0b00000000,  
                                        0b01000100, // VPP (31), 2.6 (27)
                                        0b00000000 };

    // Create a zif state to set before running the clock
    // PROG needs to be pulsed, can't be kept low the entire time.
    zif_bits_t erase_preclk;
    memcpy(erase_preclk, erase_base, 5);
    mask_prog(erase_preclk);
    
    // Create a zif state with the clock pin turned on
    zif_bits_t erase_clk;
    memcpy(erase_clk, erase_base, 5);
    mask_xtal1(erase_clk);
    
    // Enable VPP right before setting the ZIF state
    vpp_en();
    
    // Set PROG high before pulsing it low during erase
    zif_write(erase_preclk);
    __delay_us(20); // 20us might be too generous. TODO
    
    zif_clock_write(erase_base, erase_clk, 48);
    
    // We're done. Disable VPP and reset the ZIF state.
    vpp_dis();
    zif_write(zbits_null);
    
    // The client / user is expected to verify this with a read command
    // or a blank check command (TODO)
    printf("\r\nDone.\r\n");
}

void lock(unsigned char mode)
{
    /* 
     * AT89C51 lock (modes 2, 3, 4) Pinout:
     * 
     * Target   Dir     ZIF pin#    Programmer port
     * ------------------------------------------------------------------------
     * RST      <-      09          RJ4                 // (high)
     * PSEN     <-      29          RD7                 // (low)
     * ALE      <-      30          RG0                 // Pulsed write lock
     * VPP      <-      31          VPP_31              // 12v
     * VCC      <-      40          Vdd_40
     * P2.6     <-      27          RD5                 // ctrl (high)
     * P2.7     <-      28          RD6                 // ctrl (2:h, 3:h, 4:l)
     * P3.4     ->      14          RD1                 // Busy
     * P3.6     <-      16          RG1                 // ctrl (2:h, 3:l, 4:h)
     * P3.7     <-      17          RE0                 // ctrl (2:h, 3:l, 4:l)
     */
    
    // Base pin setting for erasing
    zif_bits_t lock_base = { 0b00000000,
                             0b00000001, // RST (9)
                             0b00000000,  
                             0b01000100, // VPP (31), 2.6 (27)
                             0b00000000 };
    
    zif_bits_t dir = {  0,
                        0b00100000,   // Busy signal (14)
                        0, 0, 0 };
    
    switch (mode) {
        case 2:
            mask_p2_7(lock_base);
            mask_p3_6(lock_base);
            mask_p3_7(lock_base);
            break;
        case 3:
            mask_p2_7(lock_base);
            break;
        case 4:
            mask_p3_6(lock_base);
            break;
        default:
            printf("\r\nUnknown mode %u. Valid modes are 2, 3 or 4.\r\n");
            return;
    }
    
    // Set pin direction
    dir_write(dir);
    
    // Set pins
    set_vdd(vdd);
    set_vpp(vpp);
    set_gnd(gnd);
    
    // Set voltages
    vdd_val(5); // 5.0 v - 5.2 v
    vpp_val(1); // 12.8 - 13.2
    
    // Create a zif state to set before running the clock
    // PROG needs to be pulsed, can't be kept low the entire time.
    zif_bits_t lock_preclk;
    memcpy(lock_preclk, lock_base, 5);
    mask_prog(lock_preclk);
    
    // Create a zif state with the clock pin turned on
    zif_bits_t lock_clk;
    memcpy(lock_clk, lock_base, 5);
    mask_xtal1(lock_clk);
    
    // Enable VPP right before setting the ZIF state
    vpp_en();
    
    // Set PROG high before pulsing it low during erase
    zif_write(lock_preclk);
    __delay_us(20); // 20us might be too generous. TODO
    
    zif_clock_write(lock_base, lock_clk, 48);
    
    // We're done. Disable VPP and reset the ZIF state.
    vpp_dis();
    zif_write(zbits_null);
    
    // The client / user is expected to verify this with a read command
    // or a blank check command (TODO)
    printf("\r\nDone.\r\n");
}

void verify()
{
    printf("\r\nUnimplemented.\r\n");
}

inline void eval_command(unsigned char * cmd)
{
    unsigned char * cmd_t = strtok(cmd, " ");
    switch (cmd_t[0]) {
        case 'r':
        {
            unsigned int addr  = atoi(strtok(NULL, " "));
            unsigned int range = atoi(strtok(NULL, " "));
            read(addr, range);
            break;
        }
            
        case 'w':
        {
            unsigned int addr  = atoi(strtok(NULL, " "));
            unsigned char data = atoi(strtok(NULL, " "));
            write(addr, data);
            break;
        }
        
        case 'l':
        {
            unsigned char mode = atoi(strtok(NULL, " "));
            lock(mode);
            break;
        }
            
        case 'e':
            erase();
            break;

        case 'v':
            verify();
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
