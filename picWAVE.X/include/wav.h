/******************************************************************************* 
 * 
 * File:   wav.h
 * Author: Ryan Clarke (kj6msg@arrl.net)
 * 
 * Driver for reading a WAVE file from an EEPROM. Written for a 1 Mbit EEPROM.
 * 
 * History:
 * 
 * v1.0 - First release
 * 
 ******************************************************************************/

#ifndef WAV_H
#define	WAV_H

#include <stdbool.h>
#include <stdint.h>


#ifdef	__cplusplus
extern "C" {
#endif

    // WAVE file header
    typedef struct
    {
        // Top level chunk
        uint32_t ChunkID;       // "RIFF" (big endian format)
        uint32_t ChunkSize;     // file length - 8 bytes
        uint32_t Format;        // "WAVE" (big endian format)
        
        // Format chunk
        uint32_t FmtChunkID;    // "fmt " (big endian format)
        uint32_t FmtChunkSize;  // 16 bytes
        uint16_t AudioFormat;   // WAVE file format tag
        uint16_t NumChannels;   // number of audio channels
        uint32_t SampleRate;    // samples per second
        uint32_t ByteRate;      // SampleRate * BlockAlign (bytes per second))
        uint16_t BlockAlign;    // NumChannels * BitsPerSample / 8
        uint16_t BitsPerSample; // bits per sample
        
        // Data chunk
        uint32_t DataChunkID;   // "data" (big endian format)
        uint32_t DataChunkSize; // length of data chunk in bytes
    } wav_header_t;
    
    
    // WAVE file ChunkIDs
    #define RIFF_ID 0x46464952  // "RIFF"
    #define WAVE_ID 0x45564157  // "WAVE"
    #define FMT_ID  0x20746d66  // "fmt "
    #define DATA_ID 0x61746164  // "data"
    
    
    // WAVE file AudioFormat tags
    #define WAVE_FORMAT_PCM         0x0001
    #define WAVE_FORMAT_IEEE_FLOAT  0x0003
    #define WAVE_FORMAT_ALAW        0x0006
    #define WAVE_FORMAT_MULAW       0x0007
    #define WAVE_FORMAT_EXTENSIBLE  0xfffe
    
    
    /**
     * Reads a WAVE file from an EEPROM
     */
    void wav_get_header(wav_header_t *);
    
    /**
     * Validates a WAVE file
     */
    bool wav_valid_wave(wav_header_t *);
    
    /**
     * Writes a WAVE file to an EEPROM
     */
    bool wav_write_wave(void);

#ifdef	__cplusplus
}
#endif

#endif	/* WAV_H */

