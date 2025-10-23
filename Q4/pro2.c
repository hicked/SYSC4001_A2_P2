#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(int argc, char *argv[]) {
    printf("sheeeeep\n");
    //printf("child shmid: %d\n", ((int*) argv)[1]);
    printf("%s\n", argv[0]);
    printf("int %d\n", atoi(argv[0]));
  
    int *vars = (int*) shmat(atoi(argv[0]), NULL, 0666);
    printf("sheeeeep2\n");
    while (1) {
        printf("sheeeeep3\n");
        if (vars[1] > 100) {
            
            printf("p2: %d\n", vars[1]);

        }
        usleep(1000);
    }
    /*
    //wait???
    for(; vars[1] <= 500; vars[1]++) {
        if (vars[1] % 3 == 0) {
            printf("p1: %d\n", vars[1]);
        }
        usleep(WAIT);
    }*/
}