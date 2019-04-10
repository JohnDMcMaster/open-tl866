/* 
 * File:   system.h
 * Author: William
 *
 * Created on April 16, 2018, 10:10 PM
 */

#ifndef SYSTEM_H
#define	SYSTEM_H

#ifdef	__cplusplus
extern "C" {
#endif

//16 MHz osc, but normally through PLL (96 MHz) divided by 2
//See https://github.com/ProgHQ/open-tl866/issues/74
#define _XTAL_FREQ 48000000

#ifdef	__cplusplus
}
#endif

#endif	/* SYSTEM_H */

