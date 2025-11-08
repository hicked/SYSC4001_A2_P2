#!/usr/bin/env bash

# usage: buildnrun.sh [--mac] <tracefile>
if [[ $1 == "--mac" ]]; then
    MAC=true
    shift
else
    MAC=false
fi

if [ -z "$1" ]; then
    echo "Usage: $0 [--mac] <tracefile>"
    exit 1
fi

TRACEFILE="$1"

if $MAC; then
    ./build.sh --mac
else
    ./build.sh
fi

./bin/interrupts "$TRACEFILE" vector_table.txt device_table.txt external_files.txt
if [ $? -ne 0 ]; then
    echo "Command failed!"
    exit 1
else
    echo "Command succeeded."
    cat execution.txt
fi
