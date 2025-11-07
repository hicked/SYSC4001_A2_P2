#!/usr/bin/env bash

# Build (optionally with --mac), then run with a required trace file argument

if [[ $1 == *--mac* ]]; then
    ./build.sh --mac
    shift
else
    ./build.sh
fi

# Require a trace file argument (relative to ./trace_files)
if [[ -z "$1" ]]; then
    echo "Usage: $0 [--mac] <trace_filename_in_trace_files>"
    echo "Example: $0 trace_io_heavy.txt"
    exit 1
fi

TRACE_PATH="./trace_files/$1"

if [[ ! -f "$TRACE_PATH" ]]; then
    echo "Trace file not found: $TRACE_PATH"
    echo "Available traces:"
    ls -1 ./trace_files 2>/dev/null || true
    exit 1
fi

./bin/interrupts "$TRACE_PATH" vector_table.txt device_table.txt external_files.txt

if [ $? -ne 0 ]; then
    echo "Command failed!"
else
    echo "Command succeeded."
    cat execution.txt
fi
