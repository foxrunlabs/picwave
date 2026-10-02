#!/usr/bin/env python3
###############################################################################
#
# File:   picwave.py
# Author: Ryan Clarke (kj6msg@arrl.net)
# 
# picWAVE loader. Programs the picWAVE with a new WAVE file.
# 
# History:
# 
# v1.0 - First release
# 
###############################################################################

import argparse
import math
import os
import sys
import time

import serial


# picWAVE commands
PICWAVE_ID = b'PICW'
PICWAVE_READY = b'\x10'
PICWAVE_ERROR = b'\x20'
PICWAVE_CMD_TOTAL_BLOCKS = b'\x01'
PICWAVE_CMD_SEND_BLOCK = b'\x02'

# EEPROM parameters
PAGE_SIZE = 256
EEPROM_SIZE = 131072
MAX_BLOCKS = EEPROM_SIZE / PAGE_SIZE

# UART parameters
BAUD_RATE = 1000000


# Main Function
def main():
    # Parse the command line
    parser = argparse.ArgumentParser(description='picWAVE Loader 1.0',
    								 epilog='Report bugs to: kj6msg@arrl.net')
    parser.add_argument('--device', help='serial port device file',
    					required=True)
    parser.add_argument('--version', action='version', version='%(prog)s 1.0')
    parser.add_argument('file', help='WAVE file',
    					type=argparse.FileType(mode='rb'))
    args = parser.parse_args()
    
    # Verify the WAVE file isn't too large for the picWAVE
    st = os.stat(args.file.name)
    if st.st_size > EEPROM_SIZE:
    	args.file.close()
    	sys.exit('WAVE file too large')
    
    # Compute the number of blocks for the WAVE file
    blocks = math.ceil(st.st_size / 256)
    
    # Adjust the last block to a full page size, if it isn't already
    data = args.file.read()
    data = bytearray(data)
    if st.st_size % PAGE_SIZE > 0:
    	data += bytes(PAGE_SIZE - st.st_size % PAGE_SIZE)
    
    # Create the serial port object. 1 Mbps, 8-N-1, 1 sec timeout
    try:
    	ser = serial.Serial(args.device, BAUD_RATE, timeout=1)
    except serial.SerialException:
    	args.file.close()
    	print('picwave: error:', 'serial:', "can't open", args.device,
    		  file=sys.stderr)
    	sys.exit(1)
    
    # Connect to picWAVE
    print('Connecting to picWAVE... ', end='')
    
    # Assert RTS and read four bytes for the device ID
    ser.rts = True
    device_id = ser.read(4)
    
    # Verify the device ID
    if device_id != PICWAVE_ID:
        ser.rts = False
        ser.close()
        args.file.close()
        print('unable to connect.')
        sys.exit(1)
    
    print('connected.')
    
    # Send the ready signal to the picWAVE
    ser.write(PICWAVE_READY)
    byte = ser.read(1)
    if byte != PICWAVE_CMD_TOTAL_BLOCKS:
        ser.rts = False
        ser.close()
        args.file.close()
        print('Error sending ready signal.')
        sys.exit(1)
    
    # Send block count to picWAVE
    ser.write(blocks.to_bytes(2, byteorder="little"))
    
    start_time = time.time()
    
    # Send each block to picWAVE
    for b in range(blocks):
        byte = ser.read(1)
        if byte != PICWAVE_CMD_SEND_BLOCK:
            ser.rts = False
            ser.close()
            args.file.close()
            print('Error sending block.')
            sys.exit(1)
        
        percent = 100 * ((b + 1) * PAGE_SIZE) / (blocks * PAGE_SIZE)
        print('Writing', args.file.name, progress_bar(0, blocks * PAGE_SIZE,
        	  (b + 1) * PAGE_SIZE), '{0:.0f}%'.format(percent), end = '\r')
        
        ser.write(data[(b * PAGE_SIZE):((b + 1) * PAGE_SIZE)])
    
    elapsed_time = time.time() - start_time
    
    print()
    print(args.file.name, 'written in', '{0:.1f}'.format(elapsed_time), 'seconds')
    
    # Cleanup
    ser.rts = False
    ser.close()
    args.file.close()


# Progress Bar Function
def progress_bar(min, max, value):
	bar = list('[          ]')
	percent = value / (max - min)
	
	for i in range(1, int(percent * 10) + 1):
		bar[i] = '='
	
	bar = ''.join(bar)
	return bar


# Script Catch
if __name__ == "__main__":
    main()

