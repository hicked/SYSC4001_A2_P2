#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define WAIT 50000
#define ITER 100


int main() {
    //int mult = 3;
    //volatile int i = 0;

    int shmid = shmget(IPC_PRIVATE, sizeof(int) * 2, 0666);
    printf("parent shmid: %d\n", shmid);

    char buffer[16];
    sprintf(buffer, "%d", shmid);
     
    pid_t pid = fork();

    if (pid < 0) {
       fprintf(stderr, "Fork failed");
       return 1;
    }
    else if (pid == 0) { //child (process 2)
        execl("./pro2", buffer, NULL);
        //execl("./pro2", NULL);
    }
    else { //parent (process 1)
        int *vars = shmat(shmid, NULL, 0666);
        vars[0] = 3;
        vars[1] = 0;

        for(; vars[1] <= 500; vars[1]++) {
            if (vars[1] % 3 == 0) {
                printf("p1: %d\n", vars[1]);
            }
            usleep(WAIT);
        }
        /*
        int status;
        pid = wait(&status);
        printf("process 2 ending, child pid = %d, status = %d\n", (int) pid, status);
        */
        printf("process 1 ending\n");
    }

    
    return 0;

}