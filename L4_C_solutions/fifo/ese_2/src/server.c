#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "errExit.h"

// the FIFO pathname
char *path2ServerFIFO;
// the file descriptors for the FIFO
int serverFIFO, serverFIFO_extra;

// the quit function closes the file descriptors for the FIFO,
// and removes the FIFO from the file system
void quit(int sig) {
    // the quit function is used as signal handler for the ALARM signal too.
    if (sig == SIGALRM)
        printf("<Server> Time expired!\n");

    // Closing the FIFO
    // ...

    if (serverFIFO_extra != 0 && close(serverFIFO_extra) == -1)
        errExit("close failed");

    // Removing the FIFO
    // ...

    // terminating the process
    // ...
}

int main (int argc, char *argv[]) {
    // Check command line input arguments
    // The program only wants a FIFO pathname
    if (argc != 2) {
        printf("Usage: %s fifo_pathname\n", argv[0]);
        return 1;
    }

    // read the FIFO's pathname
    path2ServerFIFO = argv[1];

    printf("<Server> making FIFO...\n");
    // make a FIFO with the following permissions:
    // user:  read, write
    // group: write
    // other: no permission
    // ...

    printf("<Server> FIFO %s created!\n", path2ServerFIFO);

    // setting a signal handler for SIGALRM signal
    // ...

    // set a 30 seconds alarm
    alarm(30);

    // Wait for clients in read-only mode. The open blocks the calling process
    // until another process opens the same FIFO in write-only mode
    printf("<Server> waiting for a client...\n");
    // ...

    // Open an extra file descriptor, so that the server does not see end-of-file
    // even if all clients closed the write end of the FIFO
    serverFIFO_extra = open(path2ServerFIFO, O_WRONLY);
    if (serverFIFO_extra == -1)
        errExit("open write-only failed");

    int bR = -1;
    int v [] = {0, 1};
    do {
        printf("<Server> waiting for vector [a,b]...\n");
        // Read 2 integers from the FIFO.
        // ...

        // remove the alarm
        alarm(0);

        // Checkthe number of bytes read from the FIFO
	    if (/*...*/)
            printf("<Server> it looks like the FIFO is broken");
        if (/*...*/)
            printf("<Server> it looks like I did not receive 2 numbers");
        else
            printf("<Server> %d is %s %d\n", v[0],
            (v[0] < v[1])? "lower than" : "greater/equals to", v[1]);

        // reset the alarm
        alarm(30);
    // iter. until the two integers are different, and the FIFO works
    } while (v[0] != v[1] && bR != -1);

    quit(0);
}