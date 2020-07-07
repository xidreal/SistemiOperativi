/// @file ackmanager.c
/// @brief Contiene l'implementazione dell' ackmanager.

#include "defines.h"
#include "err_exit.h"
#include "semaphore.h"
#include "server_lib.h" // Funzioni ausiliarie specifiche del server
#include "shared_memory.h"
#include <fcntl.h> // Flag
#include <stdio.h>  // print
#include <stdlib.h> // Malloc
#include <sys/msg.h>  // Msg_queue
#include <sys/stat.h> // Flag
#include <time.h>   // Timestamp
#include <unistd.h> 
#include "device.h"

extern AckList *AcknowledgeList;
extern int semid;

void ackmanager(){
    while (1) {

        AckMessage *ackMessage;
        AckManage ackManage[20] = {0};

        semOp(semid, SEM_ACK, -1);

        int AckLstIndex = 0;

        // CONTROLLO RICEZIONE -----------------------------------------------------------------------
        while (AckLstIndex < ACK_LIST_DIM) {

            // Se il messaggio time stamp della riga è diverso da zero, incrementa il contatore di quel message_id
            if (AcknowledgeList->Acknowledgment_List[AckLstIndex].timestamp != 0) {
                // Incrementa contatore della ricezione di quel message_id
                int counter = ackManage[(AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id)-1].counter++;
                // Memorizza indice nella tabella
                ackManage[(AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id)-1].index[counter] = AckLstIndex;

                if (ackManage[(AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id)-1].counter == 5) {

                    // MARCATURA PER ELIMINAZIONE DA ACKNOWLEDGMENTLIST (impostando timestamp a 0)
                    // Preparazione dell'ackMessage per l'invio
                    ackMessage = (AckMessage *)malloc(sizeof(AckMessage));

                    int message_id = (AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id);
                    ackMessage->mtype = message_id;
                    
                    #ifdef DEBUG
                    printf("<ACK-MANAGER> Sto eleminando il messaggio. \n");
                    #endif

                    for (int i = 0; i < 5; i++) {

                        int index = ackManage[message_id-1].index[i];

                        ackMessage->acks[i] = AcknowledgeList->Acknowledgment_List[index];
                        #ifdef DEBUG
                        printf("<ACK-MANAGER> Acks: %i | %ld | %i | %i \n", ackMessage->acks[i].message_id,
                                ackMessage->acks[i].timestamp, ackMessage->acks[i].pid_receiver, ackMessage->acks[i].pid_sender);
                        #endif
                        AcknowledgeList->Acknowledgment_List[index].timestamp = 0;
                    }

                    // ordina gli ack in ordine di arrivo 
                    sorting_date(ackMessage);

                    #ifdef DEBUG
                    printf("<ACK-MANAGER> Invio ack %ld \n", ackMessage->mtype);
                    #endif

                    // INVIO MESSAGGIO ---------------------------------------------------------------------------
                    if (msgsnd(msqid, ackMessage, sizeof(AckMessage), 0) == -1)
                        ErrExit("msgsnd failed");

                    free(ackMessage);
                    ackManage[(AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id)-1].counter = 0;
                }
            }
            AckLstIndex++;
        }
        semOp(semid, SEM_ACK, 1);
    }
}