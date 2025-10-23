#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    int i = 0;
    while (1) {
        if (i % 3 == 0) {
            printf("p2: %d\n", i);
        }
        i--;
        usleep(500000);
    }
}