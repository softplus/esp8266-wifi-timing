#!/usr/bin/env python3
# encoding: utf8

# MIT License / (C) johnmu / https://github.com/softplus/openbk-tools

import sys, time, argparse, signal, tempfile, os
import serial, serial.tools.list_ports

g_break = False
g_counter = 0
g_chars = ""

# parse commandline arguments
def parse_args():
    # speed 115200 / 921600
    description = "Serial monitor."
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('-d', '--device',
                        default='/dev/ttyUSB0',
                        help="UART device, default /dev/ttyUSB0")
    parser.add_argument('-b', '--baudrate', type=int,
                        default=115200,
                        help="UART baudrate, defaults to 115200")
    parser.add_argument('--hex', action="store_true",
                        help="Show hex")
    args = parser.parse_args()
    return args

# connect to serial port
def connect(port, baud):
    print("Trying to connect to '%s'" % port)
    serial_port = serial.Serial(port, baud, timeout=5)
    time.sleep(0.1)
    return serial_port

# read 1 character from the serial port & display 
def read_serial(serial_port, show_hex):
    global g_counter, g_chars
    ch = serial_port.read()
    if ch==b'': 
        print("no data.")
        raise
    if show_hex:
        print(ch.hex() + ' ', end='', flush=True)
        g_counter += 1
        if (ch<b' ' or ch>b'~'): ch=b'.'
        g_chars += ch.decode("utf-8")
        if g_counter % 8 == 0: print('  ', end='', flush=True)
        if g_counter % 16 ==0: 
            print(g_chars[:8] + '  ' + g_chars[8:])
            g_chars = ""
            g_counter = 0
    else:
        print(ch.decode("utf-8"), end="", flush=True)

# interrupt handler to catch abort & close port nicely
def stop_processing(sig, frame):
    global g_break
    print("Breaking ...")
    g_break = True

# check if temporary pause file exists, and if so, wait
def wait_if_needed():
    tempfilename = os.path.join(tempfile.gettempdir(), "openbk-tools-pause.tmp")
    if os.path.exists(tempfilename):
        print("Waiting for pause to complete...")
        while os.path.exists(tempfilename): time.sleep(1)

# main schboom
if __name__ == "__main__":
    args = parse_args()
    signal.signal(signal.SIGINT, stop_processing)

    serial_port = connect(args.device, args.baudrate)
    print("Reading ...")
    while not g_break:
        #wait_if_needed()

        try:
            read_serial(serial_port, args.hex)
        except: 
            print(sys.exc_info()[1])
            #time.sleep(1) # + the 5 seconds read() timeout 
            #print("Retrying port ...")
            #serial_port.close()
            if (1==2):
                while not g_break:
                    wait_if_needed()

                    try:
                        serial_port = connect(args.device, args.baudrate)
                        break
                    except:
                        print(sys.exc_info()[1])
                        time.sleep(1)

            # continue until ok or break
    # close nicely
    serial_port.close()
    