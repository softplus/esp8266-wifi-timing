#!/bin/bash

# Tries to open the serial port, displays logs.
# Parses for <field=value> items and saves them in a CSV/TSV file.
# Automatically uses venv, if it exists.
#
# Usage:
#   scripts/serial_monitor.sh PORTNAME
# eg:
#   scripts/serial_monitor.sh /dev/ttyUSB1
#
# MIT License / (C) johnmu 
# 

DEVICE=${1:-"/dev/ttyUSB1"}

if [[ ! -e "$DEVICE" ]]; then
    echo "Port $DEVICE not found, aborting."
    exit 0
fi

USE_VENV=0
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
if [[ -f "$SCRIPT_DIR/.venv/pyvenv.cfg" ]]; then
    echo "Activating venv ..."
    source $SCRIPT_DIR/.venv/bin/activate
    USE_VENV=1
fi

python $SCRIPT_DIR/serial_parse.py -d $DEVICE
echo 

if [ $USE_VENV == "1" ]; then
    echo "Deactivating venv"
    deactivate
fi
