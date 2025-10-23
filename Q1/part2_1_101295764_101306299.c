#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main() {

    pid_t pid = fork();

    if (pid < 0) {
       fprintf(stderr, "Fork failed");
       return 1;
    }
    else if (pid == 0) { //child (process 2)
        int i = 0;
        while (1) {
            printf("p2: %d\n", i);
            i++;
            usleep(500000);
        }
    }
    else { //parent (process 1)
        int i = 0;
        while (1) {
            printf("p1: %d\n", i);
            i++;
            usleep(500000);
        }
    }

    
    return 0;

}