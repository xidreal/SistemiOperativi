/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

#define DEBUGMEM

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
    
    // Liste di posizione dei PID
    Position * position_pid [5];
    for(int i = 0; i < 5; i++){
        position_pid[i] = (Position *)malloc(sizeof(Position));
    }
    
    // Da file a liste
    file_to_list(position_pid, file);

    // DEBUG: List position
    #ifdef DEBUG
    Position * current;
    for(int i = 0; i < 5; i++){
        current = position_pid[i];
        printf("DEBUG: list position (");
        while (current -> next != NULL){
                printf("%i, %i | ", current->next->x, current->next->y);
                current = current->next;
        }
        printf(")\n");
    }
    printf("\n");
    #endif

    // Crea la memoria condivisa per ospitare la Board
    int shmidBoard = alloc_shared_memory(IPC_PRIVATE, (sizeof(pid_t) * BOARD_DIM * BOARD_DIM) + sizeof(key_t));
    SharedBoard * Board = (SharedBoard *)get_shared_memory(shmidBoard, 0);

    // Inizializzo la Board
    for (int i = 0; i < BOARD_DIM; i++)
        for(int j = 0; j < BOARD_DIM; j++)
            Board->Board[i][j] = 0;
    
    int shmidAcknowledge = alloc_shared_memory(IPC_PRIVATE, (sizeof(pid_t) * ACK_LIST_DIM) + sizeof(key_t));
    AckList * AcknowledgeList = (AckList *)get_shared_memory(shmidAcknowledge, 0);

    // Crea e inizializza i semafori
    int semidBoard = semget(IPC_PRIVATE, 5, S_IRUSR | S_IWUSR);
    unsigned short semInitValBoard[] = {1, 0, 0, 0, 0};
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
    
    for(int i = 0; i < 5; i++){
        pid_t pid = fork(); 
        if (pid == -1){
            ErrExit("Fork failed");
        }

        // Codice del figlio
        if (pid == 0){
            int j = 0;
            int x;
            int y;
            
            Position * current = position_pid[i]->next;
            while (current->next != NULL){
                semOp(semidBoard, i, -1); // entra il figlio i
                //printf("x = %i, y = %i\n", current->x, current->y);
                if(j != 0)
                    Board -> Board[x][y]= 0;
                if (Board -> Board[current->x][current->y] == 0){
                    // Se è la posizione è libera
                    Board -> Board[current->x][current->y] = getpid();
                }
                x = current->x;
                y = current->y;
                current = current->next;
                j++;
                semOp(semidBoard, i+1, 1); // libera il fgilio i + 1 
            }
            
            return 0;
        }
    }

    while(1){
        sleep(2);
        // DEBUG: view Board
                #ifdef DEBUGMEM
                for (int i = 0; i < BOARD_DIM; i++){
                    for(int j = 0; j < BOARD_DIM; j++){
                        printf("%i ", Board->Board[j][i]);
                    }
                    printf("\n");
                }
                printf("\n");
                
                #endif
        semOp(semidBoard, 0, +1);
    }

    // DEBUG: Test Board
    #ifdef DEBUG
    printf("DEBUG: Test Board ");
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
    printf("\n");
    #endif
    
    // DEBUG: Test AcknowledgeList
    #ifdef DEBUG
    printf("DEBUG: Test Acknowlodgement ");
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
    printf("\n");
    #endif

    // DEBUG: Test Semaphore
    #ifdef DEBUG
    printf("DEBUG: Test semaphore ");
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
    printf("\n");
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

    return 0;
}