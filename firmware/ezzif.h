/*
EZ ZIF (ezzif) is a library to set I/O using native DIP pin numbers and simple functions
You should not use low level APIs unless you know what you are doing
*/

#ifndef EZZIF_H
#define EZZIF_H

#include "io.h"
#include <stdint.h>

#define EZZIF_D28
//#include "defines.h"

//High level API
//Tristate all outputs
//Disable all voltage rails
void ezzif_reset(void);
void ezzif_reset_vpp(void);
void ezzif_reset_vdd(void);
void ezzif_reset_gnd(void);
//Check if an error has occured and clear error status
//Returns 0 on success, otherwise an error code
//Ex: an invalid pin number
int ezzif_error(void);
//Print out lots of register states and such
void ezzif_print_debug(void);

//Low level API
//Direction buffer
extern zif_bits_t ezzif_zbd;
//Output buffer
extern zif_bits_t ezzif_zbo;


/****************************************************************************
DIP40
****************************************************************************/

//High level API
//Toggle given pin
void ezzif_toggle_d40(int n);
//Set given pin to value (0 or 1)
void ezzif_w_d40(int n, int val);
//Set source / sink on given pin
void ezzif_dir_d40(int n, int tristate);
//Set source / sink on given pin and, if output, set initial output value
void ezzif_io_d40(int n, int tristate, int val);
//Make given pin an output and set initial value
void ezzif_o_d40(int n, int val);
//Make given pin an input
void ezzif_i_d40(int n);
//Read value on given pin
int ezzif_r_d40(int n);
//Attach given pin to VDD rail and set the VDD level
void ezzif_vdd_d40(int n, int voltset);
//Attach given pin to VPP rail and set the VPP level
void ezzif_vpp_d40(int n, int voltset);
//Attach given pin to ground rail
void ezzif_gnd_d40(int n);

//Low level API
//Set given bit in zb
void zif_bit_d40(zif_bits_t zb, int n);

void ezzif_bus_dir_d40(const char *ns, unsigned len, int tristate);
void ezzif_bus_w_d40(const char *ns, unsigned len, uint16_t val);
uint16_t ezzif_bus_r_d40(const char *ns, unsigned len);



/****************************************************************************
Generic DIP
TODO: convert these to ni directly instead of DIP28 => d40 => ni
****************************************************************************/

//preprocess to propagate constants
#define ezzif_dip_40to40(n) (n)
//Pin 1 => 1
//Pin 28 => 40, etc
#define ezzif_dip_28to40(n) ((n) <= 14 ? (n) : (n) + 40 - 28)

#if defined(EZZIF_D40)
    #define ezzif_dipto40 ezzif_dip_40to40
#elif defined(EZZIF_D28)
    #define ezzif_dipto40 ezzif_dip_28to40
#else
    #error "Must define EZZIF_DIPN"
#endif

//High level API
//See d40 API for definitions
#define ezzif_toggle(n) ezzif_toggle_d40(ezzif_dipto40(n))
#define ezzif_w(n, val) ezzif_w_d40(ezzif_dipto40(n), val)
#define ezzif_dir(n, tristate) ezzif_dir_d40(ezzif_dipto40(n), tristate)
#define ezzif_io(n, tristate, val) ezzif_io_d40(ezzif_dipto40(n), tristate, val)
#define ezzif_o(n, val) ezzif_o_d40(ezzif_dipto40(n), val)
#define ezzif_i(n) ezzif_i_d40(ezzif_dipto40(n))
#define ezzif_r(n) ezzif_r_d40(ezzif_dipto40(n))
#define ezzif_vdd(n, voltset) ezzif_vdd_d40(ezzif_dipto40(n), voltset)
#define ezzif_vpp(n, voltset) ezzif_vpp_d40(ezzif_dipto40(n), voltset)
#define ezzif_gnd(n) ezzif_gnd_d40(ezzif_dipto40(n))

/*
Bus functions
ns: pin array
    LSB first
    No more than int size
len: array length
*/
//Set source / sink on given pins
void ezzif_bus_dir(const char *ns, unsigned len, int tristate);
//Set given pins to value (0 or 1)
void ezzif_bus_w(const char *ns, unsigned len, uint16_t val);
//Read value on given pins
uint16_t ezzif_bus_r(const char *ns, unsigned len);

#endif

