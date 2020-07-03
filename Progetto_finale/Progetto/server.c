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
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define SEM_ACK 5
#define SEM_BOARD 6

#define PACE_TIMER 2 // Tempo di esecuzione degli spostamenti in secondi
// #define DEBUG
#define VIEWBOARD // Visualizza spostamenti sulla board grafica 
#define REPEATPOSITION // Ripete le posizioni dei device invece di fermarsi sull'ultima
#define VIEWACKLIST // Visualizza l'acknowledgelist

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
    
    int shmidAcknowledge = alloc_shared_memory(IPC_PRIVATE, (sizeof(Acknowledgment) * ACK_LIST_DIM) + sizeof(key_t));
    AckList * AcknowledgeList = (AckList *)get_shared_memory(shmidAcknowledge, 0);

    // Crea e inizializza i semafori
    int semid = semget(IPC_CREAT, 7, IPC_CREAT | S_IRUSR | S_IWUSR);
    unsigned short semInitVal[] = {0, 0, 0, 0, 0, 1, 1, 1};
    union semun arg;
    arg.array = semInitVal;
    if (semctl(semid, 0, SETALL, arg) == -1)
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
            
            int fdFIFO; 
            // Crea la FIFO legata al Device
            // Crea la path della FIFO del device
            char path_FIFO[15+10] = "/tmp/dev_fifo.";
            char pid2string[10];
            sprintf(pid2string, "%d", getpid());
            strcat(path_FIFO, pid2string);
            // Crea la FIFO
            if (mkfifo(path_FIFO, S_IRUSR | S_IWUSR | S_IWGRP ) == -1)
                ErrExit("mkfifo failed");
            // Apri in sola lettura
            if ((fdFIFO = open(path_FIFO, O_RDONLY | O_NONBLOCK)) == -1)
                ErrExit("open failed");
            //printf("%i", fdFIFO);

            int i = 0;
            int x;
            int y;
            
            Position * current = position_pid[pid_i]->next;
            
            // Creazione della lista di messaggi del device
            Pid_message * pid_message = (Pid_message *) malloc (sizeof(Pid_message)); 
            
            
            //pid_message->next =(Pid_message *) malloc(sizeof(Pid_message));

            int step = 0;

            while (1){
                
                Pid_message * current_pid_message = pid_message;
                Pid_message * prev = pid_message;
                // Controlla che nell' Acklist sia ancora presente il messaggio con id del messaggio inviato
                // se lo è ancora: scrivi il messagio sulla lista del device e sull AckList altrimenti eliminalo dalla lista dle Device
                
                // CONTROLLO LISTA DEVICE CON ACKNOWLEDGELIST ----------------------------------------------------------
                semOp(semid, SEM_ACK, -1);
                while(current_pid_message->next != NULL){ // Scorri la lista fino alla fine e controlla i messagge_id tra AcknowledgeList e Device list

                    /*// Controllo che il message_id sia ancora in lista
                    if(current_pid_message->message.message_id != 0 &&
                        (messageID_in_Acknowledgelist(current_pid_message->message.message_id, AcknowledgeList) != 1 )){ // Se non lo è elimino il messaggio dalla lista
                        printf("<%i> Rimuovo messaggio dalla lista Device.\n", getpid());
                        
                        if(current_pid_message == prev && current_pid_message->next->next == NULL){ // Lista formata da un solo elemento
                            printf("<%i>  Svuoto la testa della lista\n", getpid());
                            pid_message =  (Pid_message *) malloc (sizeof(Pid_message));
                            free(current_pid_message);
                            current_pid_message = pid_message;
                            prev = pid_message;
                            print_list(pid_message);
                            break;
                            
                        } else if(current_pid_message == prev && current_pid_message->next->next != NULL){ // Lista di più elementi con nodo in cima alla lista
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
                    }*/

                    prev = current_pid_message; 
                    current_pid_message = current_pid_message->next;
                    
                } 
                semOp(semid, SEM_ACK, 1);
               

                // LEGGO I MESSAGGI SULLA FIFO ----------------------------------------------
                
                // READ della fifo
                int bR;
                //Acknowledgment acknowledgment;
                do{

                    Message * message = (Message *)malloc (sizeof(Message)); // Crea un buffer per il messaggio
                   
                    bR = read(fdFIFO, message, sizeof(Message)); // Legge il messaggio dalla fifo
                    // vari controlli della lettura
                    if (bR == -1){
                        ErrExit("read failed");
                    }
                    if (bR != sizeof(Message) || bR == 0){

                    } else { 
                        
                        printf("<PID %i>Legge il messaggio %i\n", getpid(), message->message_id);

                        // SCRIVI IL MESSAGGIO SULLA LISTA DEVICE ----------------------------------------------------------------
                        
                        current_pid_message->message.max_distance = message->max_distance;
                        current_pid_message->message.message_id = message->message_id;
                        current_pid_message->message.pid_receiver = message->pid_receiver;
                        current_pid_message->message.pid_sender = message->pid_sender;
                        current_pid_message->next = (Pid_message *)malloc(sizeof(Pid_message));
                        strcpy(current_pid_message->message.message, message->message);

                        // SCRIVI IL MESSAGGIO SU ACKNOWLEDGELIST ----------------------------------------------------------------

                        // Trovo la prima riga libera su Acklist
                        int AckLstIndex = 0;
                        while((AcknowledgeList -> Acknowledgment_List[AckLstIndex]).timestamp > 0 && AckLstIndex  < ACK_LIST_DIM){
                            AckLstIndex++;
                        }
                        
                        // Scrivi sull'Acknowledge_list
                        AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id = message->message_id;
                        AcknowledgeList -> Acknowledgment_List[AckLstIndex].pid_receiver = message->pid_receiver;
                        AcknowledgeList -> Acknowledgment_List[AckLstIndex].pid_sender = message->pid_sender;
                        AcknowledgeList -> Acknowledgment_List[AckLstIndex].timestamp = time(NULL);
                        //(AcknowledgeList -> Acknowledgment_List[AckLstIndex]) = acknowledgment;
                        semOp(semid, SEM_ACK, 1); 
                    }

                } while(bR > 0);


                // CONTROLLO INVIO MESSAGGIO --------------------------------------------------------------
                semOp(semid, SEM_BOARD, -1); 
                int current_x;
                int current_y;

                // cerco il la posizione del pid attuale
                for (int i = 0; i < BOARD_DIM; i++){
                    for(int j = 0; j < BOARD_DIM; j++){
                        if(Board->Board[i][j] == getpid()){
                            current_x = i; 
                            current_y = j; 
                            break;
                        }
                    }
                }
                
                // Se trovo un device sulla board che non è questo Device 
                // allora comincio a scorrere la lista dei messaggi del Device corrente
                for (int i = 0; i < BOARD_DIM; i++){
                    for(int j = 0; j < BOARD_DIM; j++){
                        if(Board->Board[i][j] != getpid() && Board->Board[i][j] != 0){
                              
                            // Inizia a scorrere la lista di messaggi
                            Pid_message * current = pid_message;
                            while (current->next != NULL){
                                
                                semOp(semid, SEM_ACK, -1); 
                               
                                //printf("<%i> Messagio %i tentativo invio a %i \n", getpid(), current->message.message_id, Board->Board[i][j]);
                                //printf("TENTATIVO SPEDIZIONE: %i:%i, %i:%i \n", current_x, current_y, i, j);
                                // Controlla che il messaggio rientri nel raggio d'azione dato dalla max_dist
                                // e che la il messaggio sia ancora in acknowledgelistmake
                                if(message_deliverbale(current_x, current_y, i, j, current->message.max_distance) &&
                                   messageID_in_Acknowledgelist(current->message.message_id, AcknowledgeList) == 1){
                                   
                                    printf("<%i> Messagio %i spedibile a %i\n", getpid(), current_pid_message->message.message_id, Board->Board[i][j] );
                                    printf("INFO SPEDIZIONE: da %i:%i a %i:%i \n", current_x, current_y, i, j);
                                    int AckLstIndex = 0;
                                    
                                    // Controlla che il messaggio non sia già stato ricevuto dal device selezionato per l'nvio
                                    while(AckLstIndex < ACK_LIST_DIM && 
                                            !(current->message.message_id == AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id &&
                                            (Board->Board[i][j] == AcknowledgeList -> Acknowledgment_List[AckLstIndex].pid_receiver))){
                                        AckLstIndex++;
                                    }

                                    // Se tutte le condizioni sono sodisfatte
                                    if (AckLstIndex == ACK_LIST_DIM){
                                        
                                        // INVIO MESSAGGIO -------------------------------------

                                        send_message(Board->Board[i][j], current->message);
                                        printf("messaggio spedito \n");
                                    }
                                    
                                }
                                current = current->next;
                                semOp(semid, SEM_ACK, 1); 
            
                            }

                        }
                    }
                }

                semOp(semid, pid_i, -1); // entra il figlio i

                // MOVIMENTO --------------------------------------

                // Azzerail valore della cella prima di spostarsi
                if(i != 0)
                    Board -> Board[x][y]= 0;
                // Scrivi il Pid del device nella posizione data dalla lista delle posizioni    
                if (Board -> Board[current->x][current->y] == 0){
                    // Se la posizione è libera allora scrivi il pid in tale poszione e aggiorna i valori di x e y
                    Board -> Board[current->x][current->y] = getpid();
                    x = current->x;
                    y = current->y;
                    i++;
                }
                
                // STAMPA OUTPUT -------------------------------------------

                // controllo che sial il 1 device per stampare la stringa iniziale
                if (pid[0] == 0)
                    printf("# Step %i: device positions ########################\n", step++);
              
                
                for (int i = 0; i < BOARD_DIM; i++){
                    for(int j = 0; j < BOARD_DIM; j++){
                        if(Board->Board[i][j] == getpid()){
                            printf("%i %i %i msgs: ", getpid(), i, j);
                            Pid_message * current = pid_message;
                            while (current->next != NULL){
                                printf("%i ", current->message.message_id);
                                current = current->next;
                            }
                            printf("\n");
                        }
                    }
                }
                
                // Controllo di che sial il 5 device per stampare la stringa finale altrimenti apro il prossimo semaforo
                if (pid[0] != 0 && pid[1] != 0 && pid[2] != 0 && pid[3] != 0 && pid[4] == 0){
                    printf("#############################################\n\n");
                } else 
                    semOp(semid, pid_i+1, 1); // libera il fgilio i + 1 


                #ifdef REPEATPOSITION
                if (current->next == NULL)
                    current = position_pid[pid_i];
                #endif
                if (current->next != NULL)
                    current = current->next;    
                
                semOp(semid, SEM_BOARD, 1); 
            }
            
            return 0;
        }
    }

    // ACK-MANAGER ------------------------------------------------

    pid_t pid_ackManager = fork();

    if (pid_ackManager == -1){
        ErrExit("Fork failed");
    }

    // Codice del Ack-Manager
    if (pid_ackManager == 0){

        while(1){
            
            AckManage ackManage[20] = {0};
            
            semOp(semid, SEM_ACK, -1);

            int AckLstIndex = 0;

            // CONTROllO RICEZIONE -----------------------------------------------------------------------

            // Scorri la lista
            while (AckLstIndex < ACK_LIST_DIM){
                
                // Se il messaggio time stamp della riga è diverso da zero, incrementa il contatore di quel message_id
                if (AcknowledgeList -> Acknowledgment_List[AckLstIndex].timestamp != 0){
                    // Incrementa contatore della ricezione di quel message_id
                    int counter = ackManage[AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id].counter++;
                    // Memorizza indice nella tablla 
                    ackManage[AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id].index[counter] = AckLstIndex;
                    
                    if (ackManage[AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id].counter == 5){
                        
                        // MARCATURA PER ELIMINAZIONE DA ACKNOWLEDGMENTLIST (impostando timestamp a 0)

                        printf("Sto eleminando il messaggio. \n");
                        int message_id = AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id;
                        for(int i = 0; i < 5; i++){
                            int index = ackManage[message_id].index[i];
                            AcknowledgeList -> Acknowledgment_List[index].timestamp = 0;
                        }
                    }
                }
                AckLstIndex++;
            }
            semOp(semid, SEM_ACK, 1); 
        }
    }
    
    // TEST
    sleep(2);
    Message this_message1;
    this_message1.pid_sender = getpid();
    this_message1.pid_receiver = pid[0];
    this_message1.message_id = 1;
    strcpy( this_message1.message, "char");
    this_message1.max_distance = 1;
    char path_FIFO[15+10] = "/tmp/dev_fifo.";
    char pid2string[10];
    sprintf(pid2string, "%d", this_message1.pid_receiver);
    strcat(path_FIFO, pid2string);
    int deviceFIFO = open(path_FIFO, O_WRONLY);
    if(deviceFIFO == -1)
        ErrExit("Open FIFO failed");
    int bW = write(deviceFIFO, &this_message1, sizeof(Message));
    if (bW == -1 || bW != sizeof(Message)){
        ErrExit("Write failed");
    }
    sleep(2);
  
    close(deviceFIFO);
    
    
    
    

   
    while(1){
        sleep(PACE_TIMER);
        
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
        
        semOp(semid, 0, 1);

        semOp(semid, SEM_ACK, -1);
        #ifdef VIEWACKLIST // stampa acknowledge list
        int AckLstIndex = 0;
        while (AckLstIndex < 20){
            printf("%i | %i | %i | %ld\n",AcknowledgeList -> Acknowledgment_List[AckLstIndex].pid_sender,
            AcknowledgeList -> Acknowledgment_List[AckLstIndex].pid_receiver, AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id ,
            AcknowledgeList -> Acknowledgment_List[AckLstIndex].timestamp);
            AckLstIndex++;
        }
        #endif
        semOp(semid, SEM_ACK, 1);

    }

    
    // TODO: da gestire in handler
    // Detach del segmento
    free_shared_memory(Board);
    // Rimozione della memoria condivisa
    remove_shared_memory(shmidBoard);

    // Rimozione semafori 
    remove_semaphore(semid);


    return 0;
}