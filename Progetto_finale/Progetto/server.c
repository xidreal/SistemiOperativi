/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "defines.h"
#include "err_exit.h"
#include "fifo.h"
#include "semaphore.h"
#include "server_lib.h" // Funzioni ausiliarie specifiche del server
#include "shared_memory.h"
#include <errno.h>
#include <fcntl.h> // Flag
#include <signal.h>
#include <stdio.h>  // print
#include <stdlib.h> // Malloc
#include <string.h>
#include <sys/msg.h>  // Msg_queue
#include <sys/stat.h> // Flag
#include <sys/wait.h>
#include <time.h>   // Timestamp
#include <unistd.h> //
#include "device.h"


#define PACE_TIMER 2 // Tempo di esecuzione degli spostamenti in secondi
// #define DEBUG

pid_t dev_pid[5];
Message messages[20];
pid_t pid_ackManager;
int shmidAcknowledge;
int shmidBoard;
int msqid;
extern Position * current_pos[5];

void signTermHandler(int sig) {
    puts("<SERVER> Chiusura dei processi figlio...");

    kill(SIGTERM, pid_ackManager);

    for (int i = 0; i < 5; i++)
        kill(SIGTERM, dev_pid[i]);

    // Detach del segmento
    free_shared_memory(Board);

    // Rimozione della memoria condivisa
    remove_shared_memory(shmidBoard);

    free_shared_memory(AcknowledgeList);

    remove_shared_memory(shmidAcknowledge);

    // Rimozione semafori
    remove_semaphore(semid);

    if(msgctl(msqid, IPC_RMID, 0)==-1)
        ErrExit("msgctl failed");

    exit(0);
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s msg_queue_key file_posizioni\n", argv[0]);
        ErrExit("Incorrect args value");
    }

    // Creo la coda di messaggi
    key_t msg_queue_key = atoi(argv[1]);
    if (msg_queue_key <= 0) {
        ErrExit("La msgkey deve essere maggiore di 0.");
        exit(1);
    }

    if ((msqid = msgget(msg_queue_key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1)
        ErrExit("msgget failed");

    printf("MSQID %i ---------------------------------------------\n", msqid);

    // Apro il file
    int file = open(argv[2], O_RDONLY);
    if (file == -1) {
        printf("File %s does not exist\n", argv[2]);
        ErrExit("File not found");
    }

    // Liste di posizione dei PID
    
    for (int i = 0; i < 5; i++) {
        position_pid[i] = (Position *)malloc(sizeof(Position));
        current_pos[i] = (Position *)malloc(sizeof(Position));
    }

    // Da file a liste di poszioni
    file_to_list(position_pid, file);

    
// DEBUG: List position
#ifdef DEBUG
    Position *current;
    for (int i = 0; i < 5; i++) {
        current = position_pid[i];
        printf("DEBUG: list position (");
        while (current->next != NULL) {
            printf("%i, %i | ", current->next->x, current->next->y);
            current = current->next;
        }
        printf(")\n");
    }
    printf("\n");
#endif

    // Crea la memoria condivisa per ospitare la Board
    shmidBoard = alloc_shared_memory(IPC_PRIVATE, (sizeof(pid_t) * BOARD_DIM * BOARD_DIM) + sizeof(key_t));
    Board = (SharedBoard *)get_shared_memory(shmidBoard, 0);

    // Inizializzo la Board
    for (int i = 0; i < BOARD_DIM; i++)
        for (int j = 0; j < BOARD_DIM; j++)
            Board->Board[i][j] = 0;

    shmidAcknowledge = alloc_shared_memory(IPC_PRIVATE, (sizeof(Acknowledgment) * ACK_LIST_DIM) + sizeof(key_t));
    AcknowledgeList = (AckList *)get_shared_memory(shmidAcknowledge, 0);

    // Crea e inizializza i semafori
    semid = semget(IPC_PRIVATE, 7, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    unsigned short semInitVal[] = {0, 0, 0, 0, 0, 1, 1};
    union semun arg;
    arg.array = semInitVal;
    if (semctl(semid, 0, SETALL, arg) == -1)
        ErrExit("semctl SETALL failed");

    // Creazione dei figli

    for (int pid_i = 0; pid_i < 5; pid_i++) {
        dev_pid[pid_i] = fork();
        if (dev_pid[pid_i] == -1) {
            ErrExit("Fork failed");
        }

        // DEVICE
        if (dev_pid[pid_i] == 0) {

            device(pid_i);
            
        }
    }

    // ACK-MANAGER ------------------------------------------------

    pid_ackManager = fork();

    if (pid_ackManager == -1) {
        ErrExit("Fork failed");
    }

    // Codice del Ack-Manager
    if (pid_ackManager == 0) {

        while (1) {

            AckMessage *ackMessage;
            //size_t mSize;
            AckManage ackManage[20] = {0};

            semOp(semid, SEM_ACK, -1);

            int AckLstIndex = 0;

            // CONTROLLO RICEZIONE -----------------------------------------------------------------------

            // Scorri la lista
            while (AckLstIndex < ACK_LIST_DIM) {

                // Se il messaggio time stamp della riga è diverso da zero, incrementa il contatore di quel message_id
                if (AcknowledgeList->Acknowledgment_List[AckLstIndex].timestamp != 0) {
                    // Incrementa contatore della ricezione di quel message_id
                    int counter = ackManage[AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id].counter++;
                    // Memorizza indice nella tablla
                    ackManage[AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id].index[counter] = AckLstIndex;

                    if (ackManage[AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id].counter == 5) {

                        // MARCATURA PER ELIMINAZIONE DA ACKNOWLEDGMENTLIST (impostando timestamp a 0)
                        // E INVIO DELL'ACK SULLA QUEUE
                        ackMessage = (AckMessage *)malloc(sizeof(AckMessage));
                        //mSize = sizeof(ackMessage) - sizeof(ackMessage->mtype);

                        int message_id = AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id;
                        ackMessage->mtype = message_id;

                        printf("<ACK-MANAGER> Sto eleminando il messaggio. \n");

                        for (int i = 0; i < 5; i++) {

                            int index = ackManage[message_id].index[i];

                            ackMessage->acks[i] = AcknowledgeList->Acknowledgment_List[index];
                            printf("<ACK-MANAGER> Acks: %i | %ld | %i | %i \n", ackMessage->acks[i].message_id,
                                   ackMessage->acks[i].timestamp, ackMessage->acks[i].pid_receiver, ackMessage->acks[i].pid_sender);

                            AcknowledgeList->Acknowledgment_List[index].timestamp = 0;
                        }

                        sorting_date(ackMessage);

                        printf("<ACK-MANAGER> Invio ack %ld \n", ackMessage->mtype);

                        if (msgsnd(msqid, ackMessage, sizeof(AckMessage), 0) == -1)
                            ErrExit("msgsnd failed");

                        free(ackMessage);
                        ackManage[AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id].counter = 0;
                    }
                }
                AckLstIndex++;
            }
            semOp(semid, SEM_ACK, 1);
        }
    }

    // Imposto il nuovo signal handler

    // set of signals (N.B. it is not initialized!)
    sigset_t mySet;
    // blocking all signals but SIGTERM
    sigfillset(&mySet);
    sigdelset(&mySet, SIGTERM);
    sigprocmask(SIG_SETMASK, &mySet, NULL);
    // set the function sigHandler as handler for the signal SIGTERM
    if (signal(SIGTERM, signTermHandler) == SIG_ERR)
        ErrExit("change signal handler failed");

    int step = 0;
    int broke;
    while (1) {
        sleep(PACE_TIMER);

        step++;

        // STAMPA OUTPUT -------------------------------------------

        // controllo che sial il 1 device per stampare la stringa iniziale

        semOp(semid, SEM_BOARD, -1);
        semOp(semid, SEM_ACK, -1);
        printf("# Step %d: device positions ########################\n", step);
           

        for (int i = 0; i < 5; i++) {
            broke = 0;
            for(int x = 0; x < BOARD_DIM && broke != 1; x++){
                for(int y = 0; y < BOARD_DIM && broke != 1; y++){
                    if(Board->Board[x][y] == dev_pid[i]){
                        printf("%i %i %i msgs:", dev_pid[i], x, y);
                        for (int j = 0; j < ACK_LIST_DIM; j++) {
                            if (AcknowledgeList->Acknowledgment_List[j].pid_receiver == dev_pid[i] &&
                                AcknowledgeList->Acknowledgment_List[j].timestamp != 0 ) {
                                printf(" %i", AcknowledgeList->Acknowledgment_List[j].message_id);
                            }
                            
                        }
                        broke = 1;
                    }
                    
                }
                
            }
            printf("\n");
        }

        printf("#############################################\n\n");
        fflush(stdout);

        semOp(semid, SEM_BOARD, 1);
        semOp(semid, SEM_ACK, 1);

// DEBUG: view Board
#ifdef VIEWBOARD
        for (int i = 0; i < BOARD_DIM; i++) {
            for (int j = 0; j < BOARD_DIM; j++) {
                printf("%i ", Board->Board[j][i]);
            }
            printf("\n");
        }
        printf("\n");
#endif
        
#ifdef VIEWACKLIST // stampa acknowledge list
        semOp(semid, SEM_ACK, -1);
        int AckLstIndex = 0;
        while (AckLstIndex < 20) {
            printf("%i | %i | %i | %ld\n", AcknowledgeList->Acknowledgment_List[AckLstIndex].pid_sender,
                   AcknowledgeList->Acknowledgment_List[AckLstIndex].pid_receiver, AcknowledgeList->Acknowledgment_List[AckLstIndex].message_id,
                   AcknowledgeList->Acknowledgment_List[AckLstIndex].timestamp);
            AckLstIndex++;
        }
        semOp(semid, SEM_ACK, 1);
#endif
        
        semOp(semid, 0, 1);
    }   

    return 0;
}
