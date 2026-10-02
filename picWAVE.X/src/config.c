/* 
 * Desc:   PIC16F1708 configuration bits
 * File:   config.c
 * Author: Ryan Clarke
 *
 * Created on August 6, 2016
 */

#include <xc.h>

// CONFIG1
#pragma config CLKOUTEN = OFF   // Clock Out: Disabled
#pragma config WDTE = SWDTEN    // Watch Dog Timer: Controlled by software
#pragma config PWRTE = ON       // Power-up Timer: Enabled
#pragma config CP = OFF         // Code Protection: Disabled
#pragma config BOREN = OFF      // Brown-out Reset: Disabled
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor: Disabled
#pragma config MCLRE = OFF      // MCLR Pin Function: Ignored
#pragma config IESO = OFF       // Internal External Switchover: Disabled
#pragma config FOSC = INTOSC    // Oscillator Selection: INTOSC

// CONFIG2
#pragma config PPS1WAY = OFF    // PPSLOCK Bit One-Way: Disabled
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset: Enabled
#pragma config LPBOR = OFF      // Low-Power Brown-out Reset: Disabled
#pragma config BORV = LO        // Brown-out Reset Voltage: Low trip point
#pragma config ZCDDIS = ON      // Zero Crossing Detector: Disabled
#pragma config LVP = ON         // Low-Voltage Programming: Enabled
#pragma config WRT = OFF        // Flash Memory Self-Write Protection: Disabled
#pragma config PLLEN = OFF      // Phase Lock Loop: Enabled by SPLLEN bit
