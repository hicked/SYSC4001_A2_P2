#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#define WAIT 100000 //50000

int main(int argc, char *argv[]) {
    //printf("%s\n", argv[0]);
    //printf("int %d\n", atoi(argv[0]));
  
    int *vars = (int*) shmat(atoi(argv[0]), NULL, 0666);

    int semid = (int) atoi(argv[1]);
    struct sembuf ctl = {0, 0, 0};

    while (vars[1] <= 500) {
        /* this works somehow...
        for(; vars[1] <= 500; vars[1]++) {
        //semop(semid, &ctl, sizeof(struct sembuf));

            //wait until semval = 0
            if (semop(semid, &ctl, 1) < 0) {
                perror("semop err");
                return 1;
            }
            
            //set semval to nonzero
            if (semctl(semid, 0, SETVAL, 1) < 0) {
                perror("semctl err");
                return 1;
            }

            if (vars[1] % vars[0] == 0) {
                printf("Process 2 (Child, PID: %d), Cycle number: %d - %d is a multiple of 3\n", getpid(), vars[1], vars[1]);
            } else {
                printf("Process 2 (Child, PID: %d), Cycle number: %d\n", getpid(), vars[1]);
            }

            //set semval to zero
            if (semctl(semid, 0, SETVAL, 0) < 0) {
                perror("semctl err");
                return 1;
            }
        
            usleep(WAIT);
        }*/

        //wait until semval = 0
        if (semop(semid, &ctl, 1) < 0) {
            perror("semop err");
            return 1;
        }
        
        //set semval to nonzero
        if (semctl(semid, 0, SETVAL, 1) < 0) {
            perror("semctl err");
            return 1;
        }


        if (vars[1] % vars[0] == 0) {
            printf("Process 2 (Child,  PID: %d), Cycle number: %d - %d is a multiple of 3\n", getpid(), vars[1], vars[1]);
        } else {
            printf("Process 2 (Child,  PID: %d), Cycle number: %d\n", getpid(), vars[1]);
        }
        //vars[1]++;
        vars[1]--;


        //set semval to zero
        if (semctl(semid, 0, SETVAL, 0) < 0) {
            perror("semctl err");
            return 1;
        }
        usleep(WAIT);
    

    }
    if (shmdt(vars) == -1) {
        perror("shmdt err");
        return 1;
    }
    exit(0);
}