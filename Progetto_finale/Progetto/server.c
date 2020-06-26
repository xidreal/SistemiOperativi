/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include <unistd.h>     //
#include <stdio.h>      // print
#include <time.h>       // Timestamp
#include <sys/msg.h>    // Msg_queue
#include <sys/stat.h>   // Flag
#include <fcntl.h>      // Flag
#include <stdlib.h>     // Malloc

//#define VIEWBOARD // Visualizza spostamenti sulla board grafica 
#define REPEATPOSITION // Ripete le posizioni dei device invece di fermarsi sull'ultima

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
    int semidAck = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
    unsigned short semInitValAck[] = {1};
    union semun argAck;
    argAck.array = semInitValAck;
    if (semctl(semidAck, 0, SETALL, argAck) == -1)
        ErrExit("semctl SETALL failed");
    
    // Creazione dei figli
    pid_t pid[5];
    for(int pid_i = 0; pid_i < 5; pid_i++){
        pid[pid_i] = fork(); 
        if (pid[pid_i] == -1){
            ErrExit("Fork failed");
        }

        // Codice del Device i-esimo
        if (pid[pid_i] == 0){
            
            // INIZIALIZZA DEVICE

            int pidFIFO; 
            // Crea la FIFO legata al Device
            // Crea la path della FIFO del device
            char path_FIFO[15+10] = "/tmp/dev_fifo.";
            char pid2string[10];
            sprintf(pid2string, "%d", getpid());
            strcat(path_FIFO, pid2string);
            // Crea la FIFO
            if (mkfifo(path_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
                ErrExit("mkfifo failed");
            // Apri in sola lettura
            if ((pidFIFO = open(path_FIFO, O_RDWR)) == -1)
                ErrExit("open failed");

            int i = 0;
            int x;
            int y;
            
            Position * current = position_pid[pid_i]->next;
            // Creazione della lista di messaggi del device
            Pid_message * pid_message = (Pid_message *) malloc (sizeof(Pid_message)); 

            while (1){

                semOp(semidBoard, pid_i, -1); // entra il figlio i
                #ifdef DEBUG
                printf("<PID %i> Passato il semaforo Board.\n", getpid());
                #endif
                semOp(semidAck, 0, -1); // entro nella sezione critica dell' Acknowlodgement List
                #ifdef DEBUG
                printf("<PID %i> Passato il semaforo Ack.\n", getpid());
                #endif
                /*
                Pid_message * current_pid_message = pid_message;
                Pid_message * prev = pid_message;
                while(current_pid_message->next != NULL){ // Scorri la lista fino alla fine e controlla i messaggi tra AcknowledgeList e Device list
                    // Controllo che il message id sia ancora in lista
                    if(messageID_in_Acknowledgelist(current_pid_message->message.message_id, AcknowledgeList) != 1){ // Se non lo è elimino il messaggio dalla lista
                        prev->next = current_pid_message->next; // Eliminazione del message in lista non più presente nell'AckowledgeList 
                        current_pid_message = prev;                                        
                    }
                    prev = current_pid_message;   
                    current_pid_message = current_pid_message->next;
                }                

                // Trovo la prima riga libera su Acklist
                Acknowledgment currentAck;
                while(&AcknowledgeList -> Acknowledgment_List[i].message_id != NULL && i < ACK_LIST_DIM)
                    i++;
                currentAck = AcknowledgeList -> Acknowledgment_List[i];

                // READ della fifo
                int bR;
                Acknowledgment acknowledgment;
                do{
                    Message message;
                    bR = read(pidFIFO, &message, sizeof(Message));
                    if (bR == -1){
                        printf("<PID %i> La FIFO potrebbe essere danneggiata", getpid());
                    }
                    if (bR != sizeof(Message) || bR == 0){
                        printf("<PID %i> I messaggi da leggere sono finiti", getpid());
                    } else {
                        acknowledgment.message_id = current_pid_message->message.message_id;
                        acknowledgment.pid_receiver = current_pid_message->message.pid_receiver;
                        acknowledgment.pid_sender = current_pid_message->message.pid_sender;
                        acknowledgment.timestamp = time(NULL);
                        currentAck = acknowledgment;
                        currentAck = AcknowledgeList -> Acknowledgment_List[i++];
                        Pid_message * newPidMessage = (Pid_message *)malloc(sizeof(Pid_message));
                        newPidMessage->message = message;
                        current_pid_message->next = newPidMessage;
                    }

                } while(bR > 0);

                i = 0; // reinizializza i*/
                
                // WRITE su acknowledge-list
                semOp(semidAck, 0, 1);

                // MOVIMENTO
                if(i != 0)
                    Board -> Board[x][y]= 0;
                if (Board -> Board[current->x][current->y] == 0){
                    // Se la posizione è libera allora scrivi il pid in tale poszione
                    Board -> Board[current->x][current->y] = getpid();
                }
              
                // TODO: binary semaphore per la board

                semOp(semidBoard, pid_i+1, 1); // libera il fgilio i + 1 
                x = current->x;
                y = current->y;
                i++;
                #ifdef REPEATPOSITION
                if (current->next == NULL)
                    current = position_pid[pid_i];
                #endif
                if (current->next != NULL)
                    current = current->next;
                
            }
            
            return 0;
        }
    }

    int step = 0;

    while(1){
        sleep(PACE_TIMER);
        // Stampa info pid
        printf("# Step %i: device positions ########################\n", step);
        for(int pid_i = 0; pid_i < 5; pid_i++)
            // ricerca del pid all'interno della Board
            for (int i = 0; i < BOARD_DIM; i++)
                for(int j = 0; j < BOARD_DIM; j++)
                    if(Board->Board[i][j] == pid[pid_i])
                        printf("%i %i %i msgs: \n", pid[pid_i], i, j);
        printf("#############################################\n\n");
        step++;
        // DEBUG: view Board
                #ifdef VIEWBOARD
                for (int i = 0; i < BOARD_DIM; i++){
                    for(int j = 0; j < BOARD_DIM; j++){
                        printf("%i ", Board->Board[j][i]);
                    }
                    printf("\n");
                }
                printf("\n");
                
                #endif
        // semOp(semidBoard, 0, +1);
        if (semctl(semidBoard, 0, SETALL, argBoard) == -1)
        ErrExit("semctl SETALL failed");
    }

    /*// DEBUG: Test Board
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
    #endif*/
    
    /* // DEBUG: Test AcknowledgeList
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
    #endif*/

    /*// DEBUG: Test Semaphore
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
    #endif*/

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