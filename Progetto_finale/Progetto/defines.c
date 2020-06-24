/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"

void file_to_list(Position * position_pid[], int file){
    // Pointer ausiliario per lo scorrimento della lista posizione
    Position * current;

    // buffer salvataggio chunk
    char buffer[BUFFER_SZ];
    
    // Lettura del file
    ssize_t bR = 0;

    // Leggo a chunks (riga del file), fintanto che vi è qualcosa da leggere
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
