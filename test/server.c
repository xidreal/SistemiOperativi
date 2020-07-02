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

    while(current_pid_message->next != NULL){ // Scorri la lista fino alla fine e controlla i messaggi tra AcknowledgeList e Device list
        // Controllo che il message_id sia ancora in lista
        if(messageID_in_Acknowledgelist(current_pid_message->message.message_id, AcknowledgeList) != 1){ // Se non lo è elimino il messaggio dalla lista
            prev->next = current_pid_message->next; // Eliminazione del message in lista non più presente nell'AckowledgeList 
            current_pid_message = prev;                                       
        }
        #ifdef DEBUG
        printf("<PID> %i->",current_pid_message->message.message_id);
        #endif
        
        prev = current_pid_message;   
        current_pid_message = current_pid_message->next;
    }    
}