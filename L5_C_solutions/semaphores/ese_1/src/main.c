#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>

#include "semaphore.h"
#include "errExit.h"

char *messages[] = {"Operativi\n", "Sistemi ", " di ", "Corso"};

int main (int argc, char *argv[]) {

    // check command line input arguments
    if (argc != 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 0;
    }

    // get how many times the programma has to write on standard out the
    // string "Corso di Sistemi Operativi\n"
    int n = atoi(argv[1]);
    if (n < 0) {
        printf("The input number must be > 0!\n");
        return 1;
    }

    // Create a semaphore set with 4 semaphores
    printf("IPC_PRIVATE: %i\n", IPC_PRIVATE); //Curiosità quanto vale IPC_PRIVATE
    int semid = semget(IPC_PRIVATE, 4, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1)
        errExit("semget failed");

    // Initialize the semaphore set with semctl
    unsigned short semInitVal[] = {0, 0, 0, 1};
    union semun arg;
    arg.array = semInitVal;
    

    if (semctl(semid, 0, SETALL, arg) == -1)
        errExit("semctl SETALL failed");

    // Generate 4 child processes:
    // child-0 prints message[0], ... child-3 prints message[3]
    for (int child = 0; child < 4; ++child) {
        pid_t pid = fork();
        // check error for fork
        if (pid == -1)
            printf("child %d not created!", child);
        // check if running process is child or parent
        else if (pid != 0) {
            // code executed only by the child
            for (int times = 0; times < n; times++) {
                // Idea: Fourth child prints and then unlock the third child
                //       Third child prints and then unlock the second child
                semOp (semid, 4-child, -1);
                
                //       First child prints and then unlock the fourth child

                // wait the i-th semaphore
                // ...

                // print the message on terminal
                printf("%s", messages[child]);
                
                // flush the standard out
                fflush(stdout);

                // unlock the (i-1)-th semaphore.
                // child is equal to 0, then the fourth child is unlocked
                if (child == 0)
                    semOp(semid, 4, 1);
                else
                    semOp (semid, 4-child-1, 1);
            }

            exit(0);
        }
    }
    // code executed only by the parent process

    // wait the termination of all child processes
    for(int i = 0; i < 4; i++)
        wait(NULL);

    // remove the created semaphore set
    if (semctl(semid, 0, IPC_RMID, 0) == -1)
        errExit("semctl IPC_RMID failed");

    return 0;
}
