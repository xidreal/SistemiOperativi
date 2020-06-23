/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include <stdio.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define DEBUG
//#define VERBOSE


int main(int argc, char * argv[]) {
    
    if (argc != 3){
        printf("Usage: %s msg_queue_key file_posizioni\n", argv[0]);
        ErrExit("Incorrect args value");
    }

    // Creo la coda di messaggi
    int msqid;
    key_t msg_queue_key = (key_t)atoi(argv[1]);
    msqid = msgget(msg_queue_key, IPC_CREAT | S_IRUSR | S_IWUSR);

    // Apro il file
    int file = open(argv[2], O_RDONLY);
    if (file == -1) {
        printf("File %s does not exist\n", argv[2]);
        ErrExit("File not found");
    }

    // Lettura del file
    ssize_t bR = 0;
    
    // Liste di posizione dei PID
    Position_head * position_pid [5];
    for(int i = 0; i < 5; i++){
        position_pid[i] = (Position_head *)malloc(sizeof(Position_head));
    }
 
    //char buffer[BUFFER_SZ + 1];
    char buffer[BUFFER_SZ];
  
    do {
        // read the file in chunks
        bR = read(file, buffer, BUFFER_SZ);
        if (bR > 0) {
            // add the character '\0' to let printf know where a
            // string ends
            #ifdef DEBUG
            buffer[bR] = '\0';
            printf("DEBUG: read file (%s)\n", buffer);
            #endif
            int j = 0;
            Position_head * current;

            for(int i = 0; i < 5; i++){
                current = position_pid[i]; // Copia della testa della lista
                while (current->next != NULL)
                    current->next = current->next->next;
                
                Position * new_position = (Position *)malloc(sizeof(Position));
                new_position->x = (int) buffer[j] - 48;
               
                j += 2;
                new_position->y = (int) buffer[j] - 48;
                
                j += 2;
                new_position-> next = NULL;
                current->next = new_position;

            }

            #ifdef DEBUG
            current = position_pid[1];
            while (current -> next != NULL){
                    printf("DEBUG: list position (%i, %i) \n", current->next->x, current->next->y);
                    current->next = current->next->next;
            }
            #endif

            
        }
    } while (bR > 0);

    // close the file descriptor
    close(file);


    // Crea la memoria condivisa per ospitare la Board
    int shmidBoard = alloc_shared_memory(IPC_PRIVATE, (sizeof(pid_t) * BOARD_DIM * BOARD_DIM) + sizeof(key_t));
    SharedBoard * Board = (SharedBoard *)get_shared_memory(shmidBoard, 0);
    
    int shmidAcknowledge = alloc_shared_memory(IPC_PRIVATE, (sizeof(pid_t) * ACK_LIST_DIM) + sizeof(key_t));
    AckList * AcknowledgeList = (AckList *)get_shared_memory(shmidAcknowledge, 0);

    // Crea e inizializza i semafori
    int semidBoard = semget(IPC_PRIVATE, 5, S_IRUSR | S_IWUSR);
    unsigned short semInitValBoard[] = {0, 0, 0, 0, 0};
    union semun argBoard;
    argBoard.array = semInitValBoard;
    if (semctl(semidBoard, 0, SETALL, argBoard) == -1)
        ErrExit("semctl SETALL failed");

    // Crea e inizializza i semafori dell'Acknowledge_list
    int semidAck = semget(IPC_PRIVATE, 2, S_IRUSR | S_IWUSR);
    unsigned short semInitValAck[] = {1, 0};
    union semun argAck;
    argAck.array = semInitValAck;
    if (semctl(semidAck, 0, SETALL, argAck) == -1)
        ErrExit("semctl SETALL failed");
    
    // DEBUG: Test Board
    #ifdef DEBUG
    Board -> Board[1][1]= 2;
    pid_t pid = fork();
    if(pid == 0){
        //int shmidBoard = alloc_shared_memory(IPC_PRIVATE, (sizeof(pid_t) * BOARD_DIM) + sizeof(key_t));
        SharedBoard * Board= (SharedBoard *)get_shared_memory(shmidBoard, 0);
        Board ->Board[1][1] = 4;
        printf("%d \n", Board -> Board[1][1]);
        return 0;
    }
    printf("%d \n", Board -> Board[1][1]);
    #endif
    
    // DEBUG: Test AcknowledgeList
    #ifdef DEBUG
    pid = fork();
    if(pid == 0){
        AcknowledgeList -> Ack[1].pid_sender = 2;
        AcknowledgeList -> Ack[1].timestamp = time(NULL);  
        semOp(semidAck, 1, 1);
        return 0;
    }
    semOp(semidAck, 1, -1);
    printf("pid_sender: %d \n", AcknowledgeList -> Ack[1].pid_sender);
    printf("timestamp: %s \n", ctime(&(AcknowledgeList -> Ack[1].timestamp)));
    #endif

    // DEBUG: Test Semaphore
    #ifdef DEBUG
    pid = fork();
    if (pid == 0){
        printf("Figlio 2 inizializzato \n");
        semOp(semidBoard, 1, -1);
        printf("Sono dentro a Board \n");
        semOp(semidBoard, 1, 1);
        semOp(semidBoard, 2, 1);
        return 0;
    }    
    pid = fork();
    if (pid == 0){
        printf("Figlio 1 inizializzato \n");
        semOp(semidBoard, 1, +1);
        printf("Semaforo incrementato \n");
        return 0;
    }
    semOp(semidBoard, 2, -1);   
    #endif

    // TODO: da gestire in handler
    // Detach del segmento
    free_shared_memory(Board);
    // Rimozione della memoria condivisa
    remove_shared_memory(shmidBoard);
    
    // Rimozione semafori scacchiera
    remove_semaphore(semidBoard);
    // Rimozione semafori acknowledge_list
    remove_semaphore(semidAck);

    /*
    Key Acknowledge_memory = Genera segmento di memoria Acknowlodge_list[100];
    pid_t Device[5];
    // Apri file posizioni
    Apri file_posizioni;*/

    return 0;
}