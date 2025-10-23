#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define WAIT 5000000
#define ITER 10

int main() {

    pid_t pid = fork();

    if (pid < 0) {
       fprintf(stderr, "Fork failed");
       return 1;
    }
    else if (pid == 0) { //child (process 2)
        usleep(WAIT);
        execl("./pro2", NULL);
    }
    else { //parent (process 1)
        for(int i = 0; i < ITER; i++) {
            if (i % 3 == 0) {
                printf("p1: %d\n", i);
            }
            usleep(WAIT / ITER);
        }
        int status;
        pid = wait(&status);
        printf("process 2 ending, child pid = %d, status = %d\n", (int) pid, status);
        printf("process 1 ending\n");
       
    }

    
    return 0;

}