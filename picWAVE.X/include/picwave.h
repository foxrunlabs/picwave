/******************************************************************************* 
 * 
 * File:   picwave.h
 * Author: Ryan Clarke (kj6msg@arrl.net)
 * 
 * Configuration constants and definitions for picWAVE. Written for the Microchip
 * PIC16F1708 microcontroller.
 * 
 * Datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/41683A.pdf
 * 
 * Pins:
 *   1 (VDD) - 3V3
 *   2 (RA5) - SCK
 *   3 (RA4) - SDO
 *   4 (RA3) - VPP
 *   5 (RC5) - SDI
 *   6 (RC4) - /SS
 *   7 (RC3) - OPA2OUT
 *   8 (RC6) - /BUTTON
 *   9 (RC7) - OPA2IN+
 *  10 (RB7) - LED1
 *  11 (RB6) - LED2
 *  12 (RB5) - OPA1IN+
 *  13 (RB4) - /RTS
 *  14 (RC2) - OPA1OUT
 *  15 (RC1) - TX
 *  16 (RC0) - RX
 *  17 (RA2) - DAC1OUT2
 *  18 (RA1) - ICSPCLK
 *  19 (RA0) - ICSPDAT
 *  20 (VSS) - GND
 * 
 * History:
 * 
 * v1.0 - First release
 * 
 ******************************************************************************/

#ifndef PICWAVE_H
#define	PICWAVE_H

#include <xc.h>
#include "25AA1024.h"


#ifdef	__cplusplus
extern "C" {
#endif
    
    // 32 MHz Fosc, used for delay functions
    #define _XTAL_FREQ 32000000

    
    // ID for serial communications
    #define PICWAVE_ID 0x57434950 // "PICW"

    
    // UART commands
    #define PICWAVE_CMD_BLOCKS 0x01
    #define PICWAVE_CMD_SEND_BLOCK 0x02
    #define PICWAVE_READY 0x10
    #define PICWAVE_ERROR 0x20
    
    
    // Pin definitions
    #define LED1    LATBbits.LATB7
    #define LED2    LATBbits.LATB6
    #define nSS     LATCbits.LATC4
    #define nBUTTON PORTCbits.RC6
    #define nRTS    PORTBbits.RB4

    
    // State machine values
    #define S_IDLE 0
    #define S_WRITE_WAVE 1
    #define S_LOAD_WAVE 2
    #define S_PLAY_WAVE 3
    #define S_DONE 4

    
    // WAVE file constants
    #define PICWAVE_MAX_FILE_SIZE   131072
    #define PICWAVE_SAMPLE_RATE     44100
    #define PICWAVE_BITS_PER_SAMPLE 8
    #define PICWAVE_NUM_CHANNELS    1
    
    #define PICWAVE_MAX_BLOCKS (PICWAVE_MAX_FILE_SIZE / E25AA1024_PAGE_SIZE)

#ifdef	__cplusplus
}
#endif

#endif	/* PICWAVE_H */

