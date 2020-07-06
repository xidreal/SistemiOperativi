#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include <unistd.h>     //
#include <stdio.h>      // print
#include <time.h>       // Timestamp
#include <sys/msg.h>    // Msg_queue
#include <sys/stat.h>   // Flag
#include <fcntl.h>      // Flag
#include <stdlib.h>     // Malloc
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#define DEBUG

typedef struct{
    long mtype;
    int test;
} AckMessage;


void signTermHandler(int sig) {
    printf("<SERVER> Chiusura dei processi figlio...");

    // terminate the server process
    exit(0);
}

int main(int argc, char * argv[]){

    
    // set of signals (N.B. it is not initialized!)
    sigset_t mySet;
    // blocking all signals but SIGTERM
    sigfillset(&mySet);
    sigdelset(&mySet, SIGTERM);
    sigprocmask(SIG_SETMASK, &mySet, NULL);
    // set the function sigHandler as handler for the signal SIGTERM
    if(signal(SIGTERM, signTermHandler) == SIG_ERR)
      ErrExit("change signal handler failed");


    while(1){
        sleep(2);
        printf("%i", getpid());
        printf("1");
    }
}