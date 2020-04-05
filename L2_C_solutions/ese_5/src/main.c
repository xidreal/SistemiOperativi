#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "errExit.h"

char *fileName = "myfile1";

int main (int argc, char *argv[]) {

    // check command line input arguments
    if (argc <= 1) {
        printf("Usage: %s cmd [arguments]\n", argv[0]);
        return 0;
    }

    switch (fork()) {
        case -1: {
            errExit("fork failed");
        }
        case 0: {
            // close the standard output and error stream
            //...

            // create a new file. The value returned by open will be 1 as it is
            // lowest available index
            int fd = //...
            if (fd == -1)
                errExit("open failed");

            // clone the file descriptor 1. The value returned by dup will be 2
            // as it is  lowest available index
            //...

            // both file descriptors 1 and 2 are pointing to the created file now.

            // replace the current process image with a new process image
            //...
            errExit("execvp failed");
        }
        default: {
            int status;
            // wait the termination of the child process
            // ...

            printf("Command %s exited with status %d\n", argv[1], WEXITSTATUS(status));
        }
    }

    return 0;
}
