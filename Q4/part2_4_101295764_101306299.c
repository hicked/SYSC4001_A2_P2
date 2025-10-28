#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define WAIT 50000
#define ITER 100



int main() {
    int shmid = shmget(IPC_PRIVATE, sizeof(int) * 2, 0666);
    printf("parent shmid: %d\n", shmid);
    if (shmid < 0) {
        perror("shmget err");
        return 1;
    }

    char buffer[16];
    sprintf(buffer, "%d", shmid);
     
    pid_t pid = fork();

    if (pid < 0) {
       //fprintf(stderr, "Fork failed");
       perror("fork err");
       return 1;
    }
    else if (pid == 0) { //child (process 2)
        execl("./pro2", buffer, NULL);
    }
    else { //parent (process 1)
        int *vars = shmat(shmid, NULL, 0666);
        vars[0] = 3;
        vars[1] = 0;

        for(; vars[1] <= 500; vars[1]++) {
            if (vars[1] % vars[0] == 0) {
                printf("Process 1 (Parent, PID: %d), Cycle number: %d - %d is a multiple of 3\n", getpid(), vars[1], vars[1]);
            } else {
                printf("Process 1 (Parent, PID: %d), Cycle number: %d\n", getpid(), vars[1]);
            }
            
            usleep(WAIT);
        }
        
        int status;
        pid = wait(&status);
        printf("process 2 ending, child pid = %d, status = %d\n", (int) pid, status);
        
        if (shmdt(vars) == -1) {
            perror("shmdt err");
            return 1;
        }
        
        if (shmctl(shmid, IPC_RMID, 0) == -1) {
            perror("shmctl err");
            return 1;
        }
        printf("process 1 ending\n");
    }

    
    return 0;

}