/* 
 * open-tl866 firmware -- Glitch mode
 */

#include "glitch.h"

inline void print_banner(void)
{
com_println("        ,/");
com_println("      ,'/");
com_println("    ,' /");
com_println("  ,'  /_____,");
com_println(".'____    ,'  open-tl866 Glitch Mode");
com_println("     /  ,'");
com_println("    / ,'");
com_println("   /,'");
com_println("  /'");
}
inline void print_help(void)
{
    com_println("\r\nCommands:\r\n  p\tPrint loaded parameters");
    com_println("  s\tStart programming + glitch routine\r\n  h\tPrint help");
    com_println("  v\tPrint version(s)\r\n");
}

inline void print_version()
{
    // All these should be defined in some config header files. TODO
    com_println("Glitch mode version: 0.0.1");
    com_println("open-tl866 lib version: UNIMPLEMENTED");
    com_println("");
}

inline void print_params()
{
    
}

inline void start()
{
    
}

inline void eval_command(unsigned char * cmd)
{
    switch (cmd[0]) {
        case 'p':
            print_params();
            break;
            
        case 's':
            start();
            break;

        case '?':
        case 'h':
            print_help();
            break;
        case 'v':
            print_version();
            break;
        default:
            com_println("\r\nError: Unknown command.");
    }
}

int glitch(void)
{
    // Wait for user interaction (press enter).
    com_readline();
    
    print_banner();
    print_help();
    enable_echo();

    unsigned char * cmd;
    while(1) {
        com_print("CMD> ");
        cmd = com_readline();
        eval_command(cmd);
    }
}
