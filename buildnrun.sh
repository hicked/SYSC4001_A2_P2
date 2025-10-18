if [[ $1 == *--mac* ]]; then
    ./build.sh --mac
else
    ./build.sh
fi
./bin/interrupts trace.txt vector_table.txt device_table.txt

if [ $? -ne 0 ]; then
    echo "Command failed!"
else
    echo "Command succeeded."
    cat execution.txt
fi
