/******************************************************************************* 
 * 
 * File:   main.c
 * Author: Ryan Clarke (kj6msg@arrl.net)
 * 
 * picWAVE WAVE file player. At the press of a button, reads a WAVE file stored
 * on a 25AA1024 SPI EEPROM (128K x 8), filters it via the onboard Op Amps, and
 * plays it through the onboard DAC. The EEPROM can be reprogrammed with a new
 * WAVE file via the UART. Written for the Microchip PIC16F1708.
 * 
 * Datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/41683A.pdf
 * 
 * History:
 * 
 * v1.0 - First release
 * 
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <xc.h>
#include <pic16f1708.h>
#include "25AA1024.h"
#include "picwave.h"
#include "wav.h"

// Global flags
char f_sample = 0;

/**
 * Main function for picWAVE.
 * @return  zero
 */
int main(void)
{
    char button, i, state = S_IDLE;
    uint8_t sample;
    uint24_t bytes;
    wav_header_t wav_header;
    
    // System Setup ////////////////////////////////////////////////////////////
    
    // Oscillator Setup
    // - System clock determined by FOSC configuration word (SCS - default)
    // - Internal 16 MHz HF clock (IRCF)
    OSCCONbits.IRCF = 0b1111;
    
    // Watchdog Timer
    // - Watchdog period 1 second (WDTPS)
    WDTCONbits.WDTPS = 0b01010;
    
    // Interrupt Setup
    // - Global enable all interrupts (GIE)
    // - Enable peripheral interrupts (PEIE)
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    
    // GPIO Setup
    // - Disable analog inputs on PORTA (ANSELA)
    // - Disable analog inputs on PORTB (ANSELB)
    // - Disable analog inputs on PORTC (ANSELC)
    // - Disable weak pull-ups on PORTA (WPUA)
    // - Disable weak pull-ups on PORTB, except RB4 [nRTS] (WPUB)
    // - Disable weak pull-ups on PORTC, except RC6 [nBUTTON] (WPUC)
    // - Global enable weak pull-ups (nWPUEN)
    // - Drive PORTB low (LATB)
    // - Set RB6 [LED2] & RB7 [LED1] as an output (TRISB)
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;
    WPUA = 0;
    WPUB = 0b00010000;
    WPUC = 0b01000000;
    OPTION_REGbits.nWPUEN = 0;
    LATB = 0;
    TRISB = 0b00110000;
    
    // SPI Setup
    // - Drive PORTA low (LATA)
    // - Set RA4 [SDO] & RA5 [SCK] as an output (TRISA)
    // - Drive PORTC low, except RC4 [/SS] (LATC)
    // - Set RC4 [/SS] as an output (TRISC)
    // - SCK = RA5 (RA5PPS, SSPCLKPPS)
    // - SDO = RA4 (RA4PPS)
    // - SDI = RC5 (SSPDATPPS)
    // - SPI master mode, 4 MHz (Fosc/4) clock (SSPM - default)
    // - Clock idle low (CKP - default)
    // - Sample data on middle of output time (SMP - default)
    // - Transmit data on falling edge (CKE)
    // - Enable MSSP (SSPEN)
    LATA = 0;
    TRISA = 0b00001111;
    LATC = 0b00010000;
    TRISCbits.TRISC4 = 0;
    RA5PPS = 0b10000;
    SSPCLKPPS = 0b00101;
    RA4PPS = 0b10010;
    SSPDATPPS = 0b10101;
    SSP1STATbits.CKE = 1;
    SSP1CON1bits.SSPEN = 1;
    
    // EUSART Setup
    // - EUSART TX = RC1 (RC1PPS)
    // - EUSART RX = RC0 (RXPPS)
    // - Baud rate = 1 Mbps (BRG16, BRGH, SP1BRG)
    // - Asynchronous mode (SYNC - default)
    // - Transmit enable (TXEN)
    // - Receive enable (CREN)
    // - Serial port enable (SPEN)
    RC1PPS = 0b10100;
    RXPPS = 0b10000;
    BAUD1CONbits.BRG16 = 1;
    TX1STAbits.BRGH = 1;
    SP1BRG = 3;
    TX1STAbits.TXEN = 1;
    RC1STAbits.CREN = 1;
    RC1STAbits.SPEN = 1;
    
    // FVR Setup
    // - Comparator FVR buffer gain = 1x (1.024V) (CDAFVR)
    FVRCONbits.CDAFVR = 0b01;
    
    // DAC Setup
    // - DAC1 positive source = FVR (DAC1PSS)
    // - DAC1 negative source = VSS (DAC1NSS - default)
    // - DAC1 output = DAC1OUT2 (DAC1OE2)
    // - DAC1 = 0.512V
    DAC1CON0bits.DAC1PSS = 0b10;
    DAC1CON0bits.DAC1OE2 = 1;
    DAC1CON1bits.DAC1R = 128;
    
    // Sallen-Key Low Pass Filter - Stage 1
    // - Non-inverting input = OPA1IN+ (OPA1CH - default)
    // - Inverting input = OPA1OUT (OPA1UG)
    // - High GBWP mode (OPA1SP)
    OPA1CONbits.OPA1UG = 1;
    OPA1CONbits.OPA1SP = 1;
    
    // Sallen-Key Low Pass Filter - Stage 2
    // - Non-inverting input = OPA2IN+ (OPA2CH - default)
    // - Inverting input = OPA2OUT (OPA2UG)
    // - High GBWP mode (OPA2SP)
    OPA2CONbits.OPA2UG = 1;
    OPA2CONbits.OPA2SP = 1;
    
    // Timer 2 Setup (sample rate timer f ~ 43956 Hz)
    // - Pre-scaler = 1:1 (T2CKPS - default)
    // - Post-scaler = 1:1 (T2OUTPS - default)
    // - Clear counter (TMR2)
    // - Period = (Fosc/4) / (pre-scaler * post-scaler * f) = 91 (PR2)
    // - Enable Timer 2 interrupt (TMR2IE)
    TMR2 = 0;
    PR2 = 91;
    PIE1bits.TMR2IE = 1;
    
    
    // Main Program Loop ///////////////////////////////////////////////////////
    while(1)
    {
        // State machine
        switch(state)
        {
            // Idle, waiting for RTS or button push
            case S_IDLE:                
                // If nRTS is asserted, then load a new WAVE file
                if(!nRTS)
                {                    
                    // Next state: write a new WAVE file
                    state = S_WRITE_WAVE;
                }
                else
                {
                    // Read the button
                    button = !nBUTTON;

                    // Debounce for 5ms
                    __delay_ms(5);

                    // If the button is pressed, load the first sample
                    if(button && !nBUTTON)
                        state = S_LOAD_WAVE;
                }
                break;
            
            // Write a new WAVE file to the EEPROM
            case S_WRITE_WAVE:
                LED2 = 1;
                
                wav_write_wave();
                
                while(!nRTS);
                
                LED2 = 0;
                    
                // Next state: idle
                state = S_IDLE;
                
                break;
            
            // Load the WAVE file header and validate it
            case S_LOAD_WAVE:
                // Load the WAVE file header into memory
                wav_get_header(&wav_header);
                
                // Validate the WAVE header for our required parameters
                if(wav_valid_wave(&wav_header) == false)
                {
                    // Blink LED1 three times at 5 Hz
                    for(i = 0; i < 3; i++)
                    {
                        LED2 = 1;
                        __delay_ms(100);
                        LED2 = 0;
                        __delay_ms(100);
                    }
                    
                    // Next state: idle
                    state = S_IDLE;
                }
                else
                {
                    // Load the byte counter
                    bytes = (uint24_t)wav_header.DataChunkSize;
                    
                    // Select the EEPROM
                    nSS = 0;

                    // Read command
                    SSP1BUF = E25AA1024_READ;

                    // Wait for receive to complete. This will be dummy data.
                    while(!SSP1STATbits.BF);
                    sample = SSP1BUF;

                    // Bits 16-23 of [address]
                    SSP1BUF = 0;

                    // Wait for receive to complete. This will be dummy data.
                    while(!SSP1STATbits.BF);
                    sample = SSP1BUF;

                    // Bits 8-15 of [address]
                    SSP1BUF = 0;

                    //Wait for receive to complete. This will be dummy data.
                    while(!SSP1STATbits.BF);
                    sample = SSP1BUF;

                    // Bits 0-7 of [address]
                    SSP1BUF = sizeof(wav_header_t);

                    // Wait for receive to complete. This will be dummy data.
                    while(!SSP1STATbits.BF);
                    sample = SSP1BUF;

                    // Send dummy data.
                    SSP1BUF = 0;

                    // Wait for receive to complete. This will be the first sample.
                    while(!SSP1STATbits.BF);
                    sample = SSP1BUF;

                    // Turn on LED1
                    LED1 = 1;
                    
                    // Enable FVR, DAC, and Op Amps
                    FVRCONbits.FVREN = 1;
                    DAC1CON0bits.DAC1EN = 1;
                    OPA1CONbits.OPA1EN = 1;
                    OPA2CONbits.OPA2EN = 1;
                    
                    // Delay 10us for Op Amp turn-on time and DAC settling time
                    __delay_us(10);

                    // Enable Timer 2 (sample rate timer)
                    T2CONbits.TMR2ON = 1;

                    // Next state: play the sound
                    state = S_PLAY_WAVE;
                }
                break;
            
            // Play the sound
            case S_PLAY_WAVE:
                // Flag is set by sample rate timer
                if(f_sample)
                {
                    // Output the next sample to the DAC and increase the byte
                    // count.
                    DAC1CON1bits.DAC1R = sample;
                    bytes--;
                    
                    // Check the byte count for end of file. If there is more
                    // left, fetch the next sample and increment the address.
                    // Otherwise, we're done.
                    if(bytes)
                    {                        
                        // Send dummy data.
                        SSP1BUF = 0;
    
                        // Wait for receive to complete. This will be the data stored at [address].
                        while(!SSP1STATbits.BF);
                        sample = SSP1BUF;
                    }
                    else
                    {
                        // Deselect EEPROM
                        nSS = 1;
                        state = S_DONE;
                    }
                    
                    // Reset the flag
                    f_sample = 0;
                }
                break;
            
            // Done playing. Reset.
            case S_DONE:
                // Let the last sample complete it's required time
                if(f_sample)
                {
                    // Disable and clear Timer 2 (sample rate timer)
                    T2CONbits.TMR2ON = 0;
                    TMR2 = 0;
                    
                    // Zero the DAC output
                    DAC1CON1bits.DAC1R = 128;

                    // Turn off LED1
                    LED1 = 0;
                    
                    // Reset the sample flag
                    f_sample = 0;
                    
                    // Disable FVR, DAC, and Op Amps
                    OPA2CONbits.OPA2EN = 0;
                    OPA1CONbits.OPA1EN = 0;
                    DAC1CON0bits.DAC1EN = 0;
                    FVRCONbits.FVREN = 0;

                    // Back to idling
                    state = S_IDLE;
                }
                break;
                
            default:
                state = S_IDLE;
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Interrupt service routine for the PIC16F1708.
 */
void __interrupt isr(void)
{
    // Timer 2 interrupt (sample rate timer)
    if(PIR1bits.TMR2IF)
    {        
        // Toggle the sample flag and reset the Timer 2 interrupt flag.
        f_sample = 1;
        PIR1bits.TMR2IF = 0;
    }
}
