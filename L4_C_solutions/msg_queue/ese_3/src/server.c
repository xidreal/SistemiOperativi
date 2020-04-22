#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>

#include "order.h"
#include "errExit.h"

// the message queue identifier
int msqid = -1;

void signTermHandler(int sig) {
    // do we have a valid message queue identifier?
     if (msqid > 0) {
      if(msgctl(msqid, IPC_RMID, NULL) == -1)
        errExit("msgctl failed");
    } else {
      printf("<Server> message queue removed successfully\n");
    }

    // terminate the server process
    exit(0);
}

int main (int argc, char *argv[]) {
    // check command line input arguments
    if (argc != 2) {
        printf("Usage: %s message_queue_key\n", argv[0]);
        exit(1);
    }

    // read the message queue key defined by user
    int msgKey = atoi(argv[1]);
    if (msgKey <= 0) {
        printf("The message queue key must be greater than zero!\n");
        exit(1);
    }

    // set of signals (N.B. it is not initialized!)
    sigset_t mySet;
    // blocking all signals but SIGTERM
    sigfillset(&mySet);
    sigdelset(&mySet, SIGTERM);
    sigprocmask(SIG_SETMASK, &mySet, NULL);
    // set the function sigHandler as handler for the signal SIGTERM
    if(signal(SIGTERM, signTermHandler) == SIG_ERR)
      errExit("change signal handler failed");

    printf("<Server> Making MSG queue...\n");
    // get the message queue, or create a new one if it does not exist
    msqid = msgget(msgKey, IPC_CREAT | S_IRUSR | S_IWUSR);

    // check functionality
    printf("<Server> sleep...\n");
    //sleep(60);
    // the process sleeps for 1 minutes. Try to send some orders
    // and check that prime users' orders are always read before
    // normal users' ones

    struct order order;
    // endless loop
    while (1) {
        // read a message from the message queue.
        errno = 0;
        size_t mSize = sizeof(struct order) - sizeof(order.mtype);
        if(msgrcv(msqid, &order, mSize, -2, IPC_NOWAIT ) == -1){
          if(errno =0 ENoMSG){
            printf("<Server> no roder here! Please contact marketing section\n");
            sleep(60);
            continue; // non credo sia essenziale
          } else {
            errExit("msgset failed");
          }
        }
        
        // print the order on standard output
        printOrder(&order);
    }

    return 0;
}
