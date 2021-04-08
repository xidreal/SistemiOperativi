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
#include "ackmanager.h"


#define PACE_TIMER 2 // Tempo di esecuzione degli spostamenti in secondi

pid_t dev_pid[5];
Message messages[20];
pid_t pid_ackManager;
int shmidAcknowledge;
int shmidBoard;
extern Position * current_pos[5];

void signTermHandler(int sig) {

    #ifdef DEBUG
    puts("<SERVER> Chiusura dei processi figlio...");
    #endif 
    
    // Terminazione dell'ackmanager
    kill(SIGTERM, pid_ackManager);
    
    // Terminazione dei Device
    for (int i = 0; i < 5; i++)
        kill(SIGTERM, dev_pid[i]);

    // Rimozione della memoria condivisa della Board
    free_shared_memory(Board);
    remove_shared_memory(shmidBoard);

    // Rimozione della memoria condivisa della AcknowledgeLists
    free_shared_memory(AcknowledgeList);
    remove_shared_memory(shmidAcknowledge);

    // Rimozione semafori
    remove_semaphore(semid);

    // Rimozione della messsage queue
    if(msgctl(msqid, IPC_RMID, 0)==-1)
        ErrExit("msgctl failed");

    // Chiusura del processo
    exit(0);
}

int main(int argc, char *argv[]) {

    // Imposto il nuovo signal handler
    sigset_t mySet;
    if(sigfillset(&mySet) == -1)
        ErrExit("sigfillset failed");
    if(sigdelset(&mySet, SIGTERM)== -1)
        ErrExit("sigdelset failed");
    if(sigprocmask(SIG_SETMASK, &mySet, NULL)== -1)
        ErrExit("sigprocmask failed");
    if (signal(SIGTERM, signTermHandler) == SIG_ERR)
        ErrExit("change signal handler failed");

    // Controllo argomenti passati
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

    // Apro il file
    int file = open(argv[2], O_RDONLY);
    if (file == -1) {
        ErrExit("File not found");
    }

    // Inizializzazione puntatori e liste di posizione dei Device
    for (int i = 0; i < 5; i++) {
        position_pid[i] = (Position *)malloc(sizeof(Position));
        current_pos[i] = (Position *)malloc(sizeof(Position));
    }

    // Trasforma il file input in liste di poszioni
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

    // Crea la memoria condivisa per ospitare l' AcknowledgeList
    shmidAcknowledge = alloc_shared_memory(IPC_PRIVATE, (sizeof(Acknowledgment) * ACK_LIST_DIM) + sizeof(key_t));
    AcknowledgeList = (AckList *)get_shared_memory(shmidAcknowledge, 0);

    // Crea e inizializza i semafori
    semid = semget(IPC_PRIVATE, 7, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    unsigned short semInitVal[] = {0, 0, 0, 0, 0, 1, 1};
    union semun arg;
    arg.array = semInitVal;
    if (semctl(semid, 0, SETALL, arg) == -1)
        ErrExit("semctl SETALL failed");

    // DEVICE -------------------------------------------------
    // Creazione dei Device
    for (int pid_i = 0; pid_i < 5; pid_i++) {
        dev_pid[pid_i] = fork(); // Memorizza i PID dei Device
        if (dev_pid[pid_i] == -1) {
            ErrExit("Fork failed");
        }
        if (dev_pid[pid_i] == 0) {
            device(pid_i); // codice Device
        }
    }

    // ACK-MANAGER ------------------------------------------------
    pid_ackManager = fork(); // Memorizza il PID dell'AckMAnager
    if (pid_ackManager == -1) {
        ErrExit("Fork failed");
    }
    if (pid_ackManager == 0) {
        ackmanager(); // codice AckManager
    }

    // SERVER --------------------------------------------------------
    int step = 0;
    int broke;
    while (1) {

        sleep(PACE_TIMER);

        semOp(semid, SEM_BOARD, -1);
        semOp(semid, SEM_ACK, -1);
        
        // Stampa output
        printf("# Step %d: device positions ########################\n", step);
        if(step == 0){
            for(int i = 0; i < 5; i++)
                printf("%i Device inizializzato\n", dev_pid[i]);
        } else {

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
        }
        printf("#############################################\n\n");
        fflush(stdout);
        
        semOp(semid, SEM_BOARD, 1);
        semOp(semid, SEM_ACK, 1);


    // DEBUG: view Board
    #ifdef DEBUG
        for (int i = 0; i < BOARD_DIM; i++) {
            for (int j = 0; j < BOARD_DIM; j++) {
                printf("%i ", Board->Board[j][i]);
            }
            printf("\n");
        }
        printf("\n");
    #endif
        
    #ifdef DEBUG // stampa acknowledge list
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
        
        semOp(semid, 0, 1); // Libera il primo Device
        step++;

    }   

    return 0;
}
