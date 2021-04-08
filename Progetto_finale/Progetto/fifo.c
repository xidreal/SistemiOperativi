/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include "defines.h"
#include "shared_memory.h"
#include <unistd.h> 
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "err_exit.h"
#include <string.h>

void send_message(pid_t pid, Message message){
    int fdFIFO; 
    // Crea la FIFO legata al Device
    // Crea la path della FIFO del device
    char path_FIFO[15+10] = "/tmp/dev_fifo.";
    char pid2string[10];
    sprintf(pid2string, "%d", pid);
    strcat(path_FIFO, pid2string);

    //modifica il pid_sender e il pid_receiver
    message.pid_receiver = pid;
    message.pid_sender = getpid();
    
    // Crea la FIFO
    // Apri in sola scrittura
    #ifdef DEBUG
    printf("<%i> %s\n", message.pid_sender, path_FIFO);
    #endif
    if ((fdFIFO = open(path_FIFO, O_WRONLY)) == -1)
        ErrExit("open failed");
    
    int bW = write (fdFIFO, &message, sizeof(Message));
    if(bW == -1 || bW != sizeof(Message)){
        ErrExit("write failed");
    }

    close(fdFIFO);
}