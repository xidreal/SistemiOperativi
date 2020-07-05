/// @file server_lib.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del server.

#include "defines.h"
#include "fcntl.h"
#include "err_exit.h"
#include "fifo.h"
#include <unistd.h>     
#include <stdio.h>      // print
#include <sys/stat.h>   // Flag
#include <fcntl.h>      // Flag
#include <stdlib.h>     // Malloc
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

extern pid_t pid[5];

void test_process(int pid_i, int message_id, int sec){
    
    sleep(sec);
    Message this_message1;
    this_message1.pid_sender = getpid();
    this_message1.pid_receiver = pid[pid_i];
    this_message1.message_id = message_id; // TEST Processo tester
    strcpy( this_message1.message, "char");
    this_message1.max_distance = 1;
    char path_FIFO[15+10] = "/tmp/dev_fifo.";
    char pid2string[10];
    sprintf(pid2string, "%d", this_message1.pid_receiver);
    strcat(path_FIFO, pid2string);
    int deviceFIFO = open(path_FIFO, O_WRONLY);
    printf("<%i> TEST invio %i\n", getpid(), this_message1.message_id);
    if(deviceFIFO == -1)
        ErrExit("Open FIFO failed");
    int bW = write(deviceFIFO, &this_message1, sizeof(Message));
    if (bW == -1 || bW != sizeof(Message)){
        ErrExit("Write failed");
    }
  
    if(close(deviceFIFO)==-1)
        ErrExit("close failed");

}

void sorting_date(AckMessage ackMessage){

    Acknowledgment swap;

    for (int i = 0 ; i < 5 - 1; i++)
    {
        for (int j = 0 ; j < 5 - i - 1; j++)
        {
        if (ackMessage.acks[j].timestamp > ackMessage.acks[j+1].timestamp) /* For decreasing order use < */
        {
            swap       = ackMessage.acks[j];
            ackMessage.acks[j]   = ackMessage.acks[j+1];
            ackMessage.acks[j+1] = swap;
        }
        }
    }
}

