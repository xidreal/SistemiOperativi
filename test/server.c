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


int main(int argc, char * args[]){

    int fdFIFO; 

    // Crea la FIFO legata al Device
    // Crea la path della FIFO del device
    char path_FIFO[15+10] = "/tmp/dev_fifo.";
    char pid2string[10];
    sprintf(pid2string, "%d", getpid());
    strcat(path_FIFO, pid2string);

    // Crea la FIFO
    if (mkfifo(path_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
        ErrExit("mkfifo failed");
    // Apri in sola lettura
    if ((fdFIFO = open(path_FIFO, O_RDWR)) == -1)
        ErrExit("open failed");
    //printf("%i", fdFIFO);
    int bR;

    do{
        #ifdef DEBUG
        printf("read fifo: %s \n", path_FIFO);
        #endif
        Message * message = (Message *)malloc (sizeof(Message));
        bR = read(fdFIFO, message, sizeof(Message));
        if (bR == -1){
            #ifdef DEBUG
            printf("<PID %i> La FIFO potrebbe essere danneggiata\n", getpid());
            #endif
            ErrExit("read failed");
        }
        if (bR != sizeof(Message) || bR == 0){
            #ifdef DEBUG
            printf("<PID %i> I messaggi da leggere sono finiti\n", getpid());
            #endif
        } else {
            printf("%f", message->max_distance);
        }

    } while(bR > 0);

    return 0;
}