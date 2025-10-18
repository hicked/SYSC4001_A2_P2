if [ ! -d "bin" ]; then
    mkdir bin
else
	rm bin/*
fi
if [[ $1 == *--mac* ]]; then
    g++ -g -std=c++17 -O0 -I . -o bin/interrupts interrupts.cpp
else 
    g++ -g -O0 -I . -o bin/interrupts interrupts.cpp
fi