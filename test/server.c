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

#define DEBUG

typedef struct{
    long mtype;
    int test;
} AckMessage;

int main(int argc, char * argv[]){

     // Creo la coda di messaggi
    int msqid;
    key_t msg_queue_key = atoi(argv[1]);
    if (msg_queue_key <= 0) {
        ErrExit("La msgkey deve essere maggiore di 0.");
        exit(1);
    }

    if((msqid = msgget(msg_queue_key, IPC_CREAT | S_IRUSR | S_IWUSR)) == -1)
        ErrExit("msgget failed");

    printf("MSQID %i ---------------------------------------------\n", msqid);
    
    AckMessage *ackMessage = (AckMessage *)malloc(sizeof(AckMessage));
    size_t msize = sizeof(ackMessage) - sizeof(ackMessage->mtype);
    ackMessage->mtype = 1; 
    ackMessage->test = 6;
    int test_msgsnd = msgsnd(msqid, ackMessage, sizeof(AckMessage), 0);
    if ( test_msgsnd== -1)
        ErrExit("msgsnd failed");
    printf("messaggio inviato %i", test_msgsnd);
}