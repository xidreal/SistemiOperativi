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
        pid = fork();
        // same code of ese_2

    }

    // get the termination status of the last created subprocess.
    do {
        res = waitpid(/*...*/);
        if (res == -1)
            errExit("waitpid failed");
    } while (res == 0);

    printf("Child %d exited, status=%d\n", /*...*/);

    return 0;
}
