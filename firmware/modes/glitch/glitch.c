/* 
 * open-tl866 firmware -- Glitch mode
 * 
 * Copyright (c) 2018 Inachis LLC and contributors. All rights reserved.
 * 
 * BSD 2-Clause "Simplified" License
 * 
 * Redistribution and use in source and binary forms,with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 
 *   - Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
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