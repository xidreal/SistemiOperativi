#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "errExit.h"

int main (int argc, char *argv[]) {

    // check command line input arguments
    if (argc != 2) {
        printf("Usage: %s numSubProcesses\n", argv[0]);
        return 0;
    }

    int n = atoi(argv[1]);
    if (n < 0) {
        printf("The number of subprocesses must be > 0!\n");
        return 1;
    }

    // init. seed for rand function
    srand(time(0));

    pid_t pid;
    for (int i = 0; i < n; ++i) {
        // generate a rand number for the subprocess
        int code = (int)(((double)rand() / RAND_MAX) * 255);

        // generate a subprocess
        pid = //...
        // both child and parent processes are here!
        if (pid == -1)
            printf("child %d not created!", i);
        else if (/*...*/) {
            printf("PID: %d , PPID: %d. Exit code: %d\n", /*...*/);
            exit(code); // <- child process must stop here!
        }
    }
    // parent process must run here!

    // get termination status of each created subprocess.
    // ...

    return 0;
}
