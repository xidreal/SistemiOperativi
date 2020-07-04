/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

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

extern pid_t pid[5];

void file_to_list(Position * position_pid[], int file){
    // Pointer ausiliario per lo scorrimento della lista posizione
    Position * current;

    // buffer salvataggio chunk
    char buffer[BUFFER_SZ];
    
    // Lettura del file
    ssize_t bR = 0;

    // Leggo a chunks (riga del file), fintanto che vi è qualcosa da leggere.
    do {
        bR = read(file, buffer, BUFFER_SZ);
        if (bR > 0) {
            
            // DEBUG: read file
            #ifdef DEBUG
            printf("DEBUG: read file | %s", buffer);
            #endif

            // variabile utilizzata come seek, per la lettura del chunks nel buffer
            int j = 0;

            for(int i = 0; i < 5; i++){
                current = position_pid[i]; // Copia della testa della lista

                while (current->next != NULL){
                    current = current->next;
                }
                
                // Creazione nodo posizione della lista
                Position * new_position = (Position *)malloc(sizeof(Position));
                new_position->x = (int) buffer[j] - 48;
                j += 2;
                new_position -> y = (int) buffer[j] - 48;
                j += 2;
                new_position -> next = NULL;

                current->next = new_position;   
            }
        }
    } while (bR > 0);

    // close the file descriptor
    close(file);
}

int message_deliverbale(int x, int y, int i, int j, int distance){
    if (sqrt(pow(x-i, 2) + pow(y-j, 2)) <= distance){
        printf("Deliverable \n");
        return 1;
    }
    
    return 0;
}

void print_list(Pid_message * head){
    printf("<%i> msgs: ", getpid());
    Pid_message * current = head;
    while (current->next != NULL){
        printf("%i ", current->message.message_id);
        current = current->next;
    }
    printf("\n");
}

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
    printf("TEST invio %i\n", this_message1.message_id);
    if(deviceFIFO == -1)
        ErrExit("Open FIFO failed");
    int bW = write(deviceFIFO, &this_message1, sizeof(Message));
    if (bW == -1 || bW != sizeof(Message)){
        ErrExit("Write failed");
    }
  
    if(close(deviceFIFO)==-1)
        ErrExit("close failed");

}

