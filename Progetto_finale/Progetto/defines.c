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
    if (sqrt(pow(x-i, 2) + pow(y-j, 2)) <= distance)
        return 1;
    
    return 0;
}

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
    // Apri in sola lettura
    printf("%s\n", path_FIFO);
    if ((fdFIFO = open(path_FIFO, O_WRONLY)) == -1)
        ErrExit("open failed");
    
    int bW = write (fdFIFO, &message, sizeof(Message));
    if(bW == -1 || bW != sizeof(Message)){
        ErrExit("write failed");
    }

    close(fdFIFO);
}
