/******************************************************************************* 
 * 
 * File:   wav.c
 * Author: Ryan Clarke (kj6msg@arrl.net)
 * 
 * Driver for reading a WAVE file from an EEPROM. Written for a 1 Mbit EEPROM.
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

static uint8_t buffer[E25AA1024_PAGE_SIZE];

/*
 * Reads a WAVE file from an EEPROM
 */
void wav_get_header(wav_header_t *wav_header)
{
    uint8_t byte, i;
    
    // Select the EEPROM
    nSS = 0;
    
    // Read command
    SSP1BUF = E25AA1024_READ;
    
    // Wait for receive to complete. This will be dummy data.
    while(!SSP1STATbits.BF);
    byte = SSP1BUF;
    
    // Bits 23-16 of EEPROM address
    SSP1BUF = 0;
    
    // Wait for receive to complete. This will be dummy data.
    while(!SSP1STATbits.BF);
    byte = SSP1BUF;
    
    // Bits 15-8 of EEPROM address
    SSP1BUF = 0;
    
    // Wait for receive to complete. This will be dummy data.
    while(!SSP1STATbits.BF);
    byte = SSP1BUF;
    
    // Bits 7-0 of EEPROM address
    SSP1BUF = 0;
    
    // Wait for receive to complete. This will be dummy data.
    while(!SSP1STATbits.BF);
    byte = SSP1BUF;
    
    for(i = 0; i < sizeof(wav_header_t); i++)
    {
        // Send dummy data.
        SSP1BUF = 0;
        
        // Wait for receive to complete. This will be the WAVE file header.
        while(!SSP1STATbits.BF);
        *((uint8_t *)wav_header + i) = SSP1BUF;
    }
    
    // De-select the EEPROM
    nSS = 1;
}


/*
 * Validates a WAVE file
 */
bool wav_valid_wave(wav_header_t *wav_header)
{
    // Overall chunk is RIFF
    if(wav_header->ChunkID != RIFF_ID)
        return false;
    
    // Verify the file is not larger than the max file size
    if((wav_header->ChunkSize + 8) > PICWAVE_MAX_FILE_SIZE)
        return false;
    
    // Is the file a WAVE file?
    if(wav_header->Format != WAVE_ID)
        return false;
    
    // First sub-chunk is the fmt chunk
    if(wav_header->FmtChunkID != FMT_ID)
        return false;
    
    // The fmt sub-chunk should be 16 bytes
    if(wav_header->FmtChunkSize != 16)
        return false;
    
    // PCM data format
    if(wav_header->AudioFormat != WAVE_FORMAT_PCM)
        return false;
    
    // Single channel audio
    if(wav_header->NumChannels != PICWAVE_NUM_CHANNELS)
        return false;
    
    // 8000 Hz sample rate
    if(wav_header->SampleRate != PICWAVE_SAMPLE_RATE)
        return false;
    
    // 8-bit data
    if(wav_header->BitsPerSample != PICWAVE_BITS_PER_SAMPLE)
        return false;
    
    // Second sub-chunk is the data chunk
    if(wav_header->DataChunkID != DATA_ID)
        return false;
    
    return wav_header->DataChunkSize ? true : false;
}


/*
 * Writes a WAVE file to an EEPROM
 */
bool wav_write_wave(void)
{
    uint8_t byte, i;
    int b, blocks = 0;
    uint24_t address = 0;
    uint32_t id = PICWAVE_ID;
        
    // Transmit picWAVE ID
    for(i = 0; i < 4; i++)
    {
        TXREG = *((uint8_t *)&id + i);
        while(!TX1STAbits.TRMT);
    }
    
    WDTCONbits.SWDTEN = 1;
    
    // Wait for a response from the loading software
    while(!PIR1bits.RCIF);
    byte = RCREG;
    
    WDTCONbits.SWDTEN = 0;
    
    // Verify the correct response
    if(byte != PICWAVE_READY)
    {
        TXREG = PICWAVE_ERROR;
        return false;
    }
    
    // Request the WAVE file size from the loading software
    TXREG = PICWAVE_CMD_BLOCKS;
    
    WDTCONbits.SWDTEN = 1;
    
    while(!PIR1bits.RCIF);
    *((uint8_t *)&blocks) = RCREG;
    while(!PIR1bits.RCIF);
    *((uint8_t *)&blocks + 1) = RCREG;
    
    WDTCONbits.SWDTEN = 0;
    
    // Verify the WAVE file will fit on the EEPROM
    if(blocks > PICWAVE_MAX_BLOCKS)
    {
        TXREG = PICWAVE_ERROR;
        return false;
    }
    
    // Receive the WAVE file in blocks
    for(b = 0; b < blocks; b++)
    {
        TXREG = PICWAVE_CMD_SEND_BLOCK;
        
        WDTCONbits.SWDTEN = 1;
        
        i = 0;
        do
        {
            while(!PIR1bits.RCIF);
            buffer[i++] = RCREG;
        } while(i);
        
        WDTCONbits.SWDTEN = 0;
        
        // Write Enable
        nSS = 0;
        
        SSP1BUF = E25AA1024_WREN;
        
        // Wait for receive to complete. This will be dummy data.
        while(!SSP1STATbits.BF);
        byte = SSP1BUF;
        
        nSS = 1;
        
        NOP();
        
        // Write Page
        nSS = 0;
        
        SSP1BUF = E25AA1024_WRITE;
        
        // Wait for receive to complete. This will be dummy data.
        while(!SSP1STATbits.BF);
        byte = SSP1BUF;
        
        SSP1BUF = *((uint8_t *)&address + 2);
        
        // Wait for receive to complete. This will be dummy data.
        while(!SSP1STATbits.BF);
        byte = SSP1BUF;
        
        SSP1BUF = *((uint8_t *)&address + 1);
        
        // Wait for receive to complete. This will be dummy data.
        while(!SSP1STATbits.BF);
        byte = SSP1BUF;
        
        SSP1BUF = *((uint8_t *)&address);
        
        // Wait for receive to complete. This will be dummy data.
        while(!SSP1STATbits.BF);
        byte = SSP1BUF;
        
        i = 0;
        do
        {
            SSP1BUF = buffer[i++];

            // Wait for receive to complete. This will be dummy data.
            while(!SSP1STATbits.BF);
            byte = SSP1BUF;
        } while(i);
        
        nSS = 1;
        
        address += E25AA1024_PAGE_SIZE;
    }
    
    return true;
}