#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(int argc, char *argv[]) {
    //printf("%s\n", argv[0]);
    //printf("int %d\n", atoi(argv[0]));
  
    int *vars = (int*) shmat(atoi(argv[0]), NULL, 0666);
    int prev = -1;
    while (vars[1] <= 500) {
        if (prev != vars[1]) {
            if (vars[1] % vars[0] == 0) {
                printf("Process 2 (Child,  PID: %d), Cycle number: %d - %d is a multiple of 3\n", getpid(), vars[1], vars[1]);
            } else {
                printf("Process 2 (Child,  PID: %d), Cycle number: %d\n", getpid(), vars[1]);
            }
            prev = vars[1];
        }

        
        //usleep(1000);
    }
    if (shmdt(vars) == -1) {
        perror("shmdt err");
        return 1;
    }
    exit(0);
    /*
    //wait???
    for(; vars[1] <= 500; vars[1]++) {
        if (vars[1] % 3 == 0) {
            printf("p1: %d\n", vars[1]);
        }
        usleep(WAIT);
    }*/
}