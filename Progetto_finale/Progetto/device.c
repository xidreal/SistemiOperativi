/// @file device.c
/// @brief Contiene l'implementazione del device.

#include "defines.h"
#include "err_exit.h"
#include "fifo.h"
#include "semaphore.h"
#include <fcntl.h> // Flag
#include <stdio.h>  // print
#include <stdlib.h> // Malloc
#include <string.h> // strcpy()
#include <sys/stat.h> // Flag
#include <time.h>   // Timestamp
#include <unistd.h> 
#include "server.h"

extern int semid;
extern AckList *AcknowledgeList;
extern Position *position_pid[5];
extern Position * current_pos[5];

void device(int dev_num){
    // INIZIALIZZA DEVICE

    // Crea la FIFO legata al Device
    int fdFIFO;
    // Crea la path della FIFO del device
    char path_FIFO[15 + 10] = "/tmp/dev_fifo.";
    char pid2string[10];
    sprintf(pid2string, "%d", getpid());
    strcat(path_FIFO, pid2string);
    // Crea e apre la FIFO
    if (mkfifo(path_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
        ErrExit("mkfifo failed");
    if ((fdFIFO = open(path_FIFO, O_RDONLY | O_NONBLOCK)) == -1)
        ErrExit("open failed");

    int i = 0;
    int x;
    int y;

    // Inizializza la poszione corrente
    current_pos[dev_num] = position_pid[dev_num]->next;

    // Inizializza la lista di messaggi del device
    Pid_message *pid_message = (Pid_message *)malloc(sizeof(Pid_message));

    Pid_message *current_pid_message;
    Pid_message *prev;
    
    while (1) {
        // entra il Device dev_num-esimo
        semOp(semid, dev_num, -1);

        current_pid_message = pid_message;
        prev = pid_message;

        
        // CONTROLLO DEVICE LIST ------------------------------------------------------------------
        // Controlla che nell' Acklist sia ancora presente il messaggio contenuto nella Device list.
        semOp(semid, SEM_ACK, -1);
        while (current_pid_message->next != NULL) { // Scorri la lista fino alla fine e controlla i messagge_id tra AcknowledgeList e Device list

            // Controllo che il message_id sia ancora in lista
            if (current_pid_message->message.message_id != 0 &&
                (messageID_in_Acknowledgelist(current_pid_message->message.message_id, AcknowledgeList) != 1)) { // Se non lo è elimino il messaggio dalla lista
                #ifdef DEBUG
                printf("<%i> Rimuovo messaggio dalla lista Device.\n", getpid());
                #endif

                if (current_pid_message == prev && current_pid_message->next->next == NULL) { // Lista formata da un solo elemento
                    free(pid_message);
                    pid_message = (Pid_message *)malloc(sizeof(Pid_message));
                    current_pid_message = pid_message;
                    prev = pid_message;
                    pid_message->next = NULL;
                    #ifdef DEBUG
                    printf("<%i>  Svuoto la testa della lista\n", getpid());
                    print_list(pid_message);
                    #endif
                    break;

                } else if (current_pid_message == prev && current_pid_message->next->next != NULL) { // Lista di più elementi con nodo da rimuoverein cima alla lista
                    free(pid_message);
                    pid_message = current_pid_message->next;
                    #ifdef DEBUG
                    printf("<%i>  Cambio la testa della lista\n", getpid());
                    print_list(pid_message);
                    #endif
                    break;

                } else { // Lista di più elementi
                    prev->next = current_pid_message->next;
                    free(current_pid_message);
                    current_pid_message = prev;
                    #ifdef DEBUG
                    printf("Elimino il nodo dalla lista\n");
                    print_list(pid_message);
                    #endif
                }
            }

            prev = current_pid_message;
            current_pid_message = current_pid_message->next;
        }
        semOp(semid, SEM_ACK, 1);

        // LEGGO I MESSAGGI SULLA FIFO -------------------------------------------------------------

        // READ della fifo
        int bR;
        do {

            Message *message = (Message *)malloc(sizeof(Message)); // Crea un buffer per il messaggio

            bR = read(fdFIFO, message, sizeof(Message)); // Legge il messaggio dalla fifo
            // vari controlli della lettura
            if (bR == -1) {
                ErrExit("read failed");
            }
            if (bR != sizeof(Message) || bR == 0) {

            } else {

                semOp(semid, SEM_ACK, -1);
                #ifdef DEBUG
                printf("<PID %i>Legge il messaggio %i\n", getpid(), message->message_id);
                #endif

                // SCRIVI IL MESSAGGIO SULLA LISTA DEVICE  ----------------------------------------------------------------
                current_pid_message->message.max_distance = message->max_distance;
                current_pid_message->message.message_id = message->message_id;
                current_pid_message->message.pid_receiver = message->pid_receiver;
                current_pid_message->message.pid_sender = message->pid_sender;
                current_pid_message->next = (Pid_message *)malloc(sizeof(Pid_message));
                strcpy(current_pid_message->message.message, message->message);

                // SCRIVI IL MESSAGGIO SU ACKNOWLEDGELIST ----------------------------------------------------------------
                // Trovo la prima riga libera su Acklist
                int AckLstIndex = 0;
                while ((AcknowledgeList->Acknowledgment_List[AckLstIndex]).timestamp > 0 && AckLstIndex < ACK_LIST_DIM) {
                    AckLstIndex++;
                }
                AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id = message->message_id;
                AcknowledgeList->Acknowledgment_List[AckLstIndex].pid_receiver = message->pid_receiver;
                AcknowledgeList->Acknowledgment_List[AckLstIndex].pid_sender = message->pid_sender;
                AcknowledgeList->Acknowledgment_List[AckLstIndex].timestamp = time(NULL);
               
                semOp(semid, SEM_ACK, 1);
            }

        } while (bR > 0);

        // INVIO MESSAGGIO --------------------------------------------------------------
        semOp(semid, SEM_BOARD, -1);
        int current_x;
        int current_y;

        // cerco il la posizione del pid attuale
        for (int i = 0; i < BOARD_DIM; i++) {
            for (int j = 0; j < BOARD_DIM; j++) {
                if (Board->Board[i][j] == getpid()) {
                    current_x = i;
                    current_y = j;
                    break;
                }
            }
        }

        // Se trovo un device sulla board che non è questo Device
        // allora comincio a scorrere la lista dei messaggi del Device corrente 
        // e controllo se il messaggio corrente sia inviabile o meno
        semOp(semid, SEM_ACK, -1);
        for (int i = 0; i < BOARD_DIM; i++) {
            for (int j = 0; j < BOARD_DIM; j++) {
                if (Board->Board[i][j] != getpid() && Board->Board[i][j] != 0) {

                    // Inizia a scorrere la lista di messaggi
                    Pid_message *current = pid_message;
                    while (current->next != NULL) {

                        // Controlla che il messaggio rientri nel raggio d'azione dato dalla max_dist
                        // e che la il messaggio sia ancora in acknowledgelist
                        if (message_deliverbale(current_x, current_y, i, j, current->message.max_distance) &&
                            messageID_in_Acknowledgelist(current->message.message_id, AcknowledgeList) == 1) {
                            #ifdef DEBUG
                            printf("<%i> Messagio %i spedibile a %i\n", getpid(), current->message.message_id, Board->Board[i][j]);
                            printf("INFO SPEDIZIONE: da %i:%i a %i:%i \n", current_x, current_y, i, j);
                            #endif
                            int AckLstIndex = 0;

                            // Controlla che il messaggio non sia già stato ricevuto dal device selezionato per l'nvio
                            while (AckLstIndex < ACK_LIST_DIM &&
                                    !(current->message.message_id == AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id &&
                                        (Board->Board[i][j] == AcknowledgeList->Acknowledgment_List[AckLstIndex].pid_receiver))) {
                                AckLstIndex++;
                            }

                            // Se tutte le condizioni sono sodisfatte
                            if (AckLstIndex == ACK_LIST_DIM) {

                                // INVIO MESSAGGIO ----------------------------------------------
                                send_message(Board->Board[i][j], current->message);
                                #ifdef DEBUG
                                printf("messaggio spedito \n");
                                #endif
                            }
                        }
                        current = current->next;
                    }
                }
            }
        }

        semOp(semid, SEM_ACK, 1);

        // MOVIMENTO --------------------------------------

        // Azzera il valore della cella prima di spostarsi
        if (i != 0)
            Board->Board[x][y] = 0;
        // Scrivi il Pid del device nella posizione data dalla lista delle posizioni
        if (Board->Board[current_pos[dev_num]->x][current_pos[dev_num]->y] == 0) {
            // Se la posizione è libera allora scrivi il pid in tale poszione e aggiorna i valori di x e y
            Board->Board[current_pos[dev_num]->x][current_pos[dev_num]->y] = getpid();
            x = current_pos[dev_num]->x;
            y = current_pos[dev_num]->y;
            i++;
        }

        // Controllo che non sia il 5 device per incremeare il semaforo successivo
        if (dev_num != 4)
            semOp(semid, dev_num + 1, 1); // libera il fgilio i + 1

    #ifdef REPEATPOSITION
        if (current_pos[dev_num]->next == NULL)
            current_pos[dev_num] = position_pid[dev_num];
    #endif
        if (current_pos[dev_num]->next != NULL)
            current_pos[dev_num] = current_pos[dev_num]->next;

        semOp(semid, SEM_BOARD, 1);
        }

}