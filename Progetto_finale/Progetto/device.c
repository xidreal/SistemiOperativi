#include "defines.h"
#include "err_exit.h"
#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"
#include <errno.h>
#include <fcntl.h> // Flag
#include <stdio.h>  // print
#include <stdlib.h> // Malloc
#include <string.h>
#include <sys/msg.h>  // Msg_queue
#include <sys/stat.h> // Flag
#include <sys/wait.h>
#include <time.h>   // Timestamp
#include <unistd.h> //
#include "server.h"

extern int semid;
extern AckList *AcknowledgeList;
extern Position *position_pid[5];
extern Position * current_pos[5];

void device(int dev_num){
    // INIZIALIZZA DEVICE

    int fdFIFO;
    // Crea la FIFO legata al Device
    // Crea la path della FIFO del device
    char path_FIFO[15 + 10] = "/tmp/dev_fifo.";
    char pid2string[10];
    sprintf(pid2string, "%d", getpid());
    strcat(path_FIFO, pid2string);
    // Crea la FIFO
    if (mkfifo(path_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
        ErrExit("mkfifo failed");
    // Apri in sola lettura
    if ((fdFIFO = open(path_FIFO, O_RDONLY | O_NONBLOCK)) == -1)
        ErrExit("open failed");
    //printf("%i", fdFIFO);

    int i = 0;
    int x;
    int y;

    current_pos[dev_num] = position_pid[dev_num]->next;

    // Creazione della lista di messaggi del device
    Pid_message *pid_message = (Pid_message *)malloc(sizeof(Pid_message));

    //pid_message->next =(Pid_message *) malloc(sizeof(Pid_message));
    Pid_message *current_pid_message;
    Pid_message *prev;
    
    while (1) {
        semOp(semid, dev_num, -1); // entra il figlio i

        current_pid_message = pid_message;
        prev = pid_message;
        // Controlla che nell' Acklist sia ancora presente il messaggio con id del messaggio inviato
        // se lo è ancora: scrivi il messagio sulla lista del device e sull AckList altrimenti eliminalo dalla lista dle Device

        // CONTROLLO LISTA DEVICE CON ACKNOWLEDGELIST ----------------------------------------------------------
        semOp(semid, SEM_ACK, -1);
        while (current_pid_message->next != NULL) { // Scorri la lista fino alla fine e controlla i messagge_id tra AcknowledgeList e Device list

            // Controllo che il message_id sia ancora in lista
            if (current_pid_message->message.message_id != 0 &&
                (messageID_in_Acknowledgelist(current_pid_message->message.message_id, AcknowledgeList) != 1)) { // Se non lo è elimino il messaggio dalla lista
                printf("<%i> Rimuovo messaggio dalla lista Device.\n", getpid());

                if (current_pid_message == prev && current_pid_message->next->next == NULL) { // Lista formata da un solo elemento
                    printf("<%i>  Svuoto la testa della lista\n", getpid());
                    free(pid_message);
                    pid_message = (Pid_message *)malloc(sizeof(Pid_message));
                    current_pid_message = pid_message;
                    prev = pid_message;
                    pid_message->next = NULL;
                    print_list(pid_message);
                    break;

                } else if (current_pid_message == prev && current_pid_message->next->next != NULL) { // Lista di più elementi con nodo da rimuoverein cima alla lista
                    printf("<%i>  Cambio la testa della lista\n", getpid());
                    free(pid_message);
                    pid_message = current_pid_message->next;
                    print_list(pid_message);
                    break;

                } else { // Lista di più elementi
                    printf("Elimino il nodo dalla lista\n");
                    prev->next = current_pid_message->next;
                    free(current_pid_message);
                    current_pid_message = prev;
                    print_list(pid_message);
                }
            }

            prev = current_pid_message;
            current_pid_message = current_pid_message->next;
        }
        semOp(semid, SEM_ACK, 1);

        // LEGGO I MESSAGGI SULLA FIFO ----------------------------------------------

        // READ della fifo
        int bR;
        //Acknowledgment acknowledgment;
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
                printf("<PID %i>Legge il messaggio %i\n", getpid(), message->message_id);

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

                // Scrivi sull'Acknowledge_list
                AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id = message->message_id;
                AcknowledgeList->Acknowledgment_List[AckLstIndex].pid_receiver = message->pid_receiver;
                AcknowledgeList->Acknowledgment_List[AckLstIndex].pid_sender = message->pid_sender;
                AcknowledgeList->Acknowledgment_List[AckLstIndex].timestamp = time(NULL);
                //(AcknowledgeList -> Acknowledgment_List[AckLstIndex]) = acknowledgment;
                semOp(semid, SEM_ACK, 1);
            }

        } while (bR > 0);

        // CONTROLLO INVIO MESSAGGIO --------------------------------------------------------------
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
        semOp(semid, SEM_ACK, -1);
        for (int i = 0; i < BOARD_DIM; i++) {
            for (int j = 0; j < BOARD_DIM; j++) {
                if (Board->Board[i][j] != getpid() && Board->Board[i][j] != 0) {

                    // Inizia a scorrere la lista di messaggi
                    Pid_message *current = pid_message;
                    while (current->next != NULL) {

                        //printf("<%i> Messagio %i tentativo invio a %i \n", getpid(), current->message.message_id, Board->Board[i][j]);
                        //printf("TENTATIVO SPEDIZIONE: %i:%i, %i:%i \n", current_x, current_y, i, j);
                        // Controlla che il messaggio rientri nel raggio d'azione dato dalla max_dist
                        // e che la il messaggio sia ancora in acknowledgelistmake
                        if (message_deliverbale(current_x, current_y, i, j, current->message.max_distance) &&
                            messageID_in_Acknowledgelist(current->message.message_id, AcknowledgeList) == 1) {

                            printf("<%i> Messagio %i spedibile a %i\n", getpid(), current->message.message_id, Board->Board[i][j]);
                            printf("INFO SPEDIZIONE: da %i:%i a %i:%i \n", current_x, current_y, i, j);
                            int AckLstIndex = 0;

                            // Controlla che il messaggio non sia già stato ricevuto dal device selezionato per l'nvio
                            while (AckLstIndex < ACK_LIST_DIM &&
                                    !(current->message.message_id == AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id &&
                                        (Board->Board[i][j] == AcknowledgeList->Acknowledgment_List[AckLstIndex].pid_receiver))) {
                                AckLstIndex++;
                            }

                            // Se tutte le condizioni sono sodisfatte
                            if (AckLstIndex == ACK_LIST_DIM) {

                                // INVIO MESSAGGIO -------------------------------------

                                send_message(Board->Board[i][j], current->message);
                                printf("messaggio spedito \n");
                            }
                        }
                        current = current->next;
                    }
                }
            }
        }

        semOp(semid, SEM_ACK, 1);

        // MOVIMENTO --------------------------------------

        // Azzerail valore della cella prima di spostarsi
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