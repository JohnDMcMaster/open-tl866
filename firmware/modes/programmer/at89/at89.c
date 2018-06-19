#include "at89.h"

static zif_bits_t zbits_null = {0, 0, 0, 0, 0};
static zif_bits_t gnd        = {0, 0, 8, 0, 0};
static zif_bits_t vdd        = {0, 0, 4, 0, 128};
static zif_bits_t vpp        = {0, 0, 0, 64, 0};

inline void print_banner(void)
{
    com_println("   | |");
    com_println(" ==[+]==  open-tl866 Programmer Mode (AT89)");
    com_println("   | |");
}
inline void print_help(void)
{
    com_println("\r\nCommands:\r\n  r <ADDR (hex)> [RANGE (hex)]\tRead from target");
    com_println("  w <ADDR (hex)> <BYTE (hex)>\tWrite to target");
    com_println("  e\t\t\t\tErase target");
    com_println("  l <MODE (int)>\t\tSet lock bits to MODE");
    com_println("  s\t\t\t\tPrint signature bytes");
    com_println("  b\t\t\t\tBlank check");
    com_println("  T\t\t\t\tRun some tests");
    com_println("  h\t\t\t\tPrint help\r\n  v\t\t\t\tPrint version(s)");
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

inline void mask_p2_7(zif_bits_t op_base)
{
    op_base[3] |= 0x8;
}

inline void mask_p3_6(zif_bits_t op_base)
{
    op_base[1] |= 0x80;
}

inline void mask_p3_7(zif_bits_t op_base)
{
    op_base[2] |= 0x1;
}

inline void mask_xtal1(zif_bits_t op_base)
{
    op_base[2] |= 0x4;
}

inline void mask_prog(zif_bits_t op_base)
{
    op_base[3] |= 0x20;
}

inline void mask_addr(zif_bits_t op_base, unsigned int addr)
{
    op_base[0] = addr & 0xFF;
    op_base[2] |= (addr >> 8) << 4;
}

inline void mask_data(zif_bits_t op_base, unsigned char data)
{
    op_base[3] |= (data & 0x80);
    op_base[4] |= (invert_bit_endianness(data & 0x7f) >> 1);
}

inline unsigned char zif_to_data(zif_bits_t zif_state)
{
    // Filter the zif_bits response into a char byte with P0 bits
                   // Trim non-data ZIF pins    // Set the LSB of data byte
    unsigned char byte = (zif_state[4] << 1) | !! (zif_state[3] & (1 << 7));

    // Invert bit-endianness
    return invert_bit_endianness(byte);
}

// Flip clock pin directly from TL866
inline void pin_flip_clock()
{
    PORTE ^= (1 << 2);
}

inline void print_zif_state(zif_bits_t op)
{
    com_println("");
    com_println("01-08 09-16 17-24 25-32 33-40");
    for (unsigned char i = 0; i < 5; i++) {
        printf("%02X    ", op[i]);
    }
    com_println("");
}

inline void clock_write(zif_bits_t op, unsigned int cycles)
{
    zif_write(op);
    for(unsigned int i = 0; i <= cycles; i++) {
        pin_flip_clock();
        __delay_us(1);
        pin_flip_clock();
        __delay_us(1);
    }
}

// Very slow, but useful for prototyping when other
// pins need to be changed alongside the clock
inline void zif_clock_write(zif_bits_t op_template, zif_bits_t op_clk,
                            unsigned int cycles
                            )
{
    for(unsigned char i = 0; i <= cycles; i++) {
        zif_write(op_template);
        zif_write(op_clk);
    }
}

unsigned char read_byte(unsigned int addr)
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
    zif_bits_t response    = { 0, 0, 0, 0, 0 };

    // Base pin setting for reading
    zif_bits_t read_base = { 0b00000000,
                             0b10000001,   // 3.6 ctrl (16), RST (9)
                             0b00000001,   // 3.7 ctrl (17) 
                             0b01100000,   // VPP (31), PROG (30)
                             0b00000000 };

    zif_bits_t read_clk;

    // Mask in the address bits to the appropriate pins
    mask_addr(read_base, addr);

    // Give the clock on/off states to zif_clock_write(..) and loop 48 cycles
    clock_write(read_base, 48);

    // Read the current pin state (to read in the requested byte)
    zif_read(response);

    // We're done with the byte. Turn off all outputs.
    zif_write(zbits_null);
    
    return zif_to_data(response);
}

void read(unsigned int addr, unsigned int range)
{    
    printf("%03X ", addr);

    
    if (!range) { range = 1; } else {com_println("");}
    for (unsigned int byte_idx = 0; byte_idx < range; byte_idx++) {
        printf("%02X ", read_byte(addr + byte_idx));
    }
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
     * PRO      <-      30          RG0                     // Pulsed prog.
     * VPP      <-      31          VPP_31                  // 12v
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
    
    printf("Writing %02X at %03X... ", data, addr);

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
   
    // Enable VPP right before setting the ZIF state
    vpp_en();
    
    // Set PROG high before pulsing it low during programming
    zif_write(write_preclk);
    __delay_us(20); // 20us might be too generous. TODO

    clock_write(write_base, 48);

    // We're done. Disable VPP and reset the ZIF state.
    vpp_dis();
    zif_write(zbits_null);
    
    // The client / user is expected to verify this with a read command.
    printf("done.");
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
     * VPP      <-      31          VPP_31                  // 12v
     * VCC      <-      40          Vdd_40
     * P2.6     <-      27          RD5                     // ctrl (high)
     * P2.7     <-      28          RD6                     // ctrl (low)
     * P3.4     ->      14          RD1                     // Busy
     * P3.6     <-      16          RG1                     // ctrl (low)
     * P3.7     <-      17          RE0                     // ctrl (low)
     */
    
    printf("Erasing... ");
    
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
    
    // Enable VPP right before setting the ZIF state
    vpp_en();
    
    // Set PROG high before pulsing it low during erase
    zif_write(erase_preclk);
    __delay_us(20);
    
    clock_write(erase_base, 48);
    
    // Erase function requires 10ms prog pulse
    __delay_ms(10);
    
    // We're done. Disable VPP and reset the ZIF state.
    vpp_dis();
    zif_write(zbits_null);
    
    // The client / user is expected to verify this with a read command
    // or a blank check command (TODO)
    printf("done.");
}

// Does not work yet.
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
    
    printf("Locking with mode %u... ", mode);
    
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
            printf("Invalid mode %u. Valid modes are 2, 3 or 4.", mode);
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
    
    // Enable VPP right before setting the ZIF state
    vpp_en();
    
    // Set PROG high before pulsing it low during erase
    zif_write(lock_preclk);
    __delay_us(20); // 20us might be too generous. TODO
    
    clock_write(lock_base, 48);
    
    // We're done. Disable VPP and reset the ZIF state.
    vpp_dis();
    zif_write(zbits_null);
    
    // The client / user is expected to verify this with a read command
    // or a blank check command (TODO)
    printf("done.");
}

unsigned char read_sig(unsigned int offset)
{
   // TODO. Implements the signature reading routine as described in the
   // datasheet. Would be a good precheck before doign read/write/erase ops.
    
     /* 
     * AT89C51 Read Signature Pinout:
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
     * P3.6     <-      16          RG1                     // ctrl (low)
     * P3.7     <-      17          RE0                     // ctrl (low)
     */
    
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
    zif_bits_t response  = { 0, 0, 0, 0, 0 };

    // Base pin setting for reading
    zif_bits_t signature_base = { 0b00000000,
                             0b00000001,   // RST (9)
                             0b00000000,
                             0b01100000,   // VPP (31), PROG (30)
                             0b00000000 };

    zif_bits_t signature_clk;
    
    // Mask in the address bits to the appropriate pins
    mask_addr(signature_base, 0x30 + offset);

    // Give the clock on/off states to zif_clock_write(..) and loop 48 cycles
    clock_write(signature_base, 48);

    // Read the current pin state (to read in the requested byte)
    zif_read(response);

    // We're done with the byte. Turn off all outputs.
    zif_write(zbits_null);
    
    return zif_to_data(response);
}

bool sig_check()
{
    if (read_sig(0) != 0x1E || read_sig(1) != 0x51 || read_sig(2) != 0xFF)
        return false;
    return true;
}

bool blank_check()
{
    printf("Performing a blank-check... ");
    unsigned char data = 0;
    for (unsigned int addr = 0; addr < 0xFFF; addr++) {
        printf("%03X", addr);
        data = read_byte(addr);
        if (data != 0xFF) {
            printf("\b\b\bdone. %03X set to byte %02X. Target is not blank.",
                    addr, data);
            return false;
        }
        printf("\b\b\b");
    }
    printf("done. Target is blank.");
    return true;
}

void self_test()
{
    printf("Testing first 255 bytes...\r\n");
    erase();
    com_println("");
    for(unsigned int addr = 0; addr < 0xff; addr++) {
        write(addr, addr);
        com_println("");
    }
    read(0,0xFF);
    com_println("");
    printf("Testing last 255 bytes...\r\n");
    for(unsigned int addr = 0xF00; addr <= 0xFFF; addr++) {
        write(addr, addr - 0xF00);
        com_println("");
    }
    read(0xF00, 0xFF);
    printf("\r\nTesting last byte...\r\n");
    write(0xFFF, 0);
    com_println("");
    read(0xFFF, 0);
    printf("\r\ndone.");
}

inline void eval_command(unsigned char * cmd)
{
    unsigned char * cmd_t = strtok(cmd, " ");
    switch (cmd_t[0]) {
        case 'r':
        {
            if (!sig_check()) {
                printf("Could not detect an AT89C51. Ignoring command.");
                break;
            }
            
            unsigned int addr  = xtoi(strtok(NULL, " "));
            unsigned int range = xtoi(strtok(NULL, " "));
            read(addr, range);
            break;
        }
            
        case 'w':
        {
            if (!sig_check()) {
                printf("Could not detect an AT89C51. Ignoring command.");
                break;
            }
            
            unsigned int addr  = xtoi(strtok(NULL, " "));
            unsigned char data = xtoi(strtok(NULL, " "));
            write(addr, data);
            break;
        }
        
        case 'l':
        {
            if (!sig_check()) {
                printf("Could not detect an AT89C51. Ignoring command.");
                break;
            }
            
            unsigned char mode = atoi(strtok(NULL, " "));
            lock(mode);
            break;
        }
            
        case 'e':
            if (!sig_check()) {
                printf("Could not detect an AT89C51. Ignoring command.");
                break;
            }
            
            erase();
            break;

        case 's':
            printf("(0x30) Manufacturer: %02X\r\n", read_sig(0));
            printf("(0x31) Model:        %02X\r\n", read_sig(1));
            printf("(0x32) VPP Voltage:  %02X\r\n", read_sig(2));
            break;
            
        case 'T':
            if (!sig_check()) {
                printf("Could not detect an AT89C51. Ignoring command.");
                break;
            }
            
            self_test();
            break;
            
        case 'b':
            if (!sig_check()) {
                printf("Could not detect an AT89C51. Ignoring command.");
                break;
            }
            
            blank_check();
            break;
            
        case '?':
        case 'h':
            print_help();
            break;
        case 'V':
            print_version();
            break;
        default:
            printf("Error: Unknown command.");
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
        printf("\r\nCMD> ");
        cmd = com_readline();
        com_println("");
        eval_command(cmd);
    }
    
}
