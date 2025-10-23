#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int i = 0;
    while (1) {
        if (i % 3 == 0) {
            printf("p2: %d\n", i);
        }


        if (i < -500) { //lower than -500
            exit(0);
        }
        i--;
        usleep(10000);
    }
}