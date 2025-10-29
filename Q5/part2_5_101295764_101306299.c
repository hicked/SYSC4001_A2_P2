#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#define WAIT 50000

int main() {
    //semaphore
    int semid = semget(IPC_PRIVATE, 1, 0666);
    printf("semaphore semid: %d\n", semid);
    if (semid < 0) {
        perror("semget err");
        return 1;
    }

    char sem_str[16];
    sprintf(sem_str, "%d", semid);
    
    struct sembuf ctl = {0, 0, 0};

    
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
    //printf("semctl get val %d\n", semctl(semid, 0, GETVAL));

    //shared memory
    int shmid = shmget(IPC_PRIVATE, sizeof(int) * 2, 0666);
    printf("parent shmid: %d\n", shmid);
    if (shmid < 0) {
        perror("shmget err");
        return 1;
    }

    char shm_str[16];
    sprintf(shm_str, "%d", shmid);

    int *vars = shmat(shmid, NULL, 0666);
    vars[0] = 3;
    vars[1] = 0;

    //set semval back to 0
    if (semctl(semid, 0, SETVAL, 0) < 0) {
        perror("semctl err");
        return 1;
    }
    
    pid_t pid = fork();

    if (pid < 0) {
       //fprintf(stderr, "Fork failed");
       perror("fork err");
       return 1;
    }
    else if (pid == 0) { //child (process 2)
        execl("./pro2", shm_str, sem_str, NULL);
    }
    else { //parent (process 1)

        /*
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
                printf("Process 1 (Parent, PID: %d), Cycle number: %d - %d is a multiple of 3\n", getpid(), vars[1], vars[1]);
            } else {
                printf("Process 1 (Parent, PID: %d), Cycle number: %d\n", getpid(), vars[1]);
            }

            //set semval to zero
            if (semctl(semid, 0, SETVAL, 0) < 0) {
                perror("semctl err");
                return 1;
            }
            
            usleep(WAIT);*/

        while (vars[1] <= 500) {
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
                printf("Process 1 (Parent, PID: %d), Cycle number: %d - %d is a multiple of 3\n", getpid(), vars[1], vars[1]);
            } else {
                printf("Process 1 (Parent, PID: %d), Cycle number: %d\n", getpid(), vars[1]);
            }

            //set semval to zero
            if (semctl(semid, 0, SETVAL, 0) < 0) {
                perror("semctl err");
                return 1;
            }
            
            vars[1]++;
            //usleep(WAIT);
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