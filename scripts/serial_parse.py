#!/usr/bin/env python3
# encoding: utf8

# MIT License / (C) johnmu / https://github.com/softplus/openbk-tools

import sys, time, argparse, signal, tempfile, os
import serial, serial.tools.list_ports

g_break = False
g_counter = 0
g_chars = ""
g_fieldnames = []
g_rowdata = {}
g_csv_filename = ""
g_field_filename = ""

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
    parser.add_argument('-f', '--fields', type=str,
                        default="__fields.txt",
                        help="File for fieldname tracking")
    parser.add_argument('-s', '--statfile', type=str,
                        default="__stats.csv",
                        help="File for statistics on fields")
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
    return ch.decode("utf-8")

# read file for fieldnames
def read_field_file(filename):
    global g_fieldnames
    tmp = []
    if os.path.exists(filename):
        with open(filename) as f:
            tmp = f.readlines()
    g_fieldnames = [x.strip() for x in tmp]

# save file for fieldnames
def save_field_file(filename):
    global g_fieldnames
    with open(filename, "w") as f:
        f.writelines([x + "\n" for x in g_fieldnames])

# append row to CSV data with fields
def append_csv_data(filename, data_row):
    global g_fieldnames
    if not os.path.exists(filename):
        with open(filename, "w") as f:
            f.write("\t".join(g_fieldnames)+"\n")
    with open(filename, "a") as f:
        data = []
        for fn in g_fieldnames:
            if fn in data_row:
                data.append(data_row[fn])
            else:
                data.append("")
        f.write("\t".join(data)+"\n")
    # done

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

# parse the recent buffer, looking for <field=value>
def parse_buffer(buffer):
    global g_fieldnames, g_rowdata, g_csv_filename, g_field_filename
    if buffer=="<start>": 
        g_rowdata = {}
        return
    if buffer=="<complete>":
        append_csv_data(g_csv_filename, g_rowdata)
        return
    if buffer[0]!="<" or buffer[-1]!=">": return
    items = buffer[1:][:-1].split("=")
    if len(items)!=2 or not items[1]: return
    if items[0] not in g_fieldnames:
        g_fieldnames.append(items[0])
        save_field_file(g_field_filename)
    g_rowdata[items[0]] = items[1]

# main schboom
if __name__ == "__main__":
    args = parse_args()
    signal.signal(signal.SIGINT, stop_processing)
    g_csv_filename = args.statfile
    g_field_filename = args.fields
    read_field_file(g_field_filename)
    serial_port = connect(args.device, args.baudrate)
    buffer = ""
    while not g_break:
        try:
            ch = read_serial(serial_port, args.hex)
            buffer += ch
            if ch=='<': buffer="<"
            if ch=='>': 
                parse_buffer(buffer)
                buffer=""
        except: 
            print(sys.exc_info()[1])
            break
    # close nicely
    serial_port.close()
    