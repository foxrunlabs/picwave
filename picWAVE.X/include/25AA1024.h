/******************************************************************************* 
 * 
 * File:   eeprom.h
 * Author: Ryan Clarke (kj6msg@arrl.net)
 * 
 * Commands for the Microchip 25AA1024 SPI EEPROM (128K x 8).
 * 
 * Datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/20001836J.pdf
 * 
 * History:
 * 
 * v1.0 - First release
 * 
 ******************************************************************************/

#ifndef EEPROM_H
#define	EEPROM_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    #define E25AA1024_PAGE_SIZE 256
    
    // 25AA1024 SPI EEPROM Commands
    #define E25AA1024_WRSR  0x01
    #define E25AA1024_WRITE 0x02    
    #define E25AA1024_READ  0x03
    #define E25AA1024_WRDI  0x04
    #define E25AA1024_RDSR  0x05
    #define E25AA1024_WREN  0x06
    #define E25AA1024_PE    0x42
    #define E25AA1024_RDID  0xab
    #define E25AA1024_DPD   0xb9
    #define E25AA1024_CE    0xc7
    #define E25AA1024_SE    0xd8

#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */

