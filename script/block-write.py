#!/usr/bin/env python
'''
use python script to test real transfer speed of usb cdc device.
'''
import serial
import time


def main():
    ser = serial.Serial()
    ser.baudrate = 8000000
    # ser.port = 'COM5'
    ser.port = '/dev/tty.usbmodem3955387534391'
    ser.parity = serial.PARITY_NONE
    ser.stopbits = serial.STOPBITS_ONE
    ser.timeout = 0.1

    packet_size = 4096
    packet = bytearray()
    for i in range(packet_size):
        packet.append(0xa5)
    ser.open()
    start =time.time()
    loop_size = 4096
    for i in range(loop_size):
        ser.write(packet)
    end = time.time()
    print('Running time: %s Seconds'%(end-start))
    print('Real Speed %f Mbits' % (packet_size*loop_size/(end-start)/1024.0/1024.0*8.0))


if __name__ == "__main__":
    main()
    
