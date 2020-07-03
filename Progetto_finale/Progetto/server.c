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

//#define DEBUG
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

                #ifdef DEBUG
                printf("<PID %i> Passato il semaforo Ack.\n", getpid());
                #endif

                Pid_message * current_pid_message = pid_message;
                Pid_message * prev = pid_message;
                // Controlla che nell' Acklist sia ancora presente il messaggio con id del messaggio inviato
                // se lo è ancora: scrivi il messagio sulla lista del device e sull AckList altrimenti eliminalo dalla lista dle Device
                
                // entro nella sezione critica dell' Acknowlodgement List
                #ifdef DEBUG
                semOp(semidAck, 0, -1);
                printf("<PID %i> LIST: \n", getpid());
                #endif
                
                semOp(semidAck, 0, -1);
                while(current_pid_message->next != NULL){ // Scorri la lista fino alla fine e controlla i messagge_id tra AcknowledgeList e Device list

                    // Controllo che il message_id sia ancora in lista
                    if(current_pid_message->message.message_id != 0 &&
                        (messageID_in_Acknowledgelist(current_pid_message->message.message_id, AcknowledgeList) != 1 )){ // Se non lo è elimino il messaggio dalla lista
                        printf("<%i> Rimuovo messaggio dalla lista Device.\n", getpid());
                        
                        
                        if(current_pid_message == prev && current_pid_message->next->next == NULL){ // Lista formata da un solo elemento
                            printf("<%i>  Svuoto la testa della lista\n", getpid());
                            pid_message =  (Pid_message *) malloc (sizeof(Pid_message));
                            free(current_pid_message);
                            current_pid_message = pid_message;
                            prev = pid_message;
                            break;
                            
                        } else if(current_pid_message == prev && current_pid_message->next->next != NULL){ // Lista di più elementi con nodo in cima alla lista
                            printf("<%i>  Cambio la testa della lista\n", getpid()); 
                            free(pid_message);
                            pid_message = current_pid_message->next;
                            break;

                        } else { // Lista di più elementi
                            printf("Elimino il nodo dalla lista\n"); 
                            prev->next = current_pid_message->next; 
                            free(current_pid_message);
                            current_pid_message = prev;  
                        }                  
                    }
                    #ifdef DEBUG
                    printf("<PID> %i->",current_pid_message->message.message_id);
                    #endif
                    
                    prev = current_pid_message; 
                    current_pid_message = current_pid_message->next;
                    
                } 
                semOp(semidAck, 0, 1);
                //Acknowledgment * currentAck = &AcknowledgeList -> Acknowledgment_List[j];

                //j = 0; // reinizializza i
                
                // READ della fifo
                int bR;
                //Acknowledgment acknowledgment;
                do{

                    #ifdef DEBUG
                    printf("read fifo: %s \n", path_FIFO);
                    #endif
                    
                    Message * message = (Message *)malloc (sizeof(Message)); // Crea un buffer per il messaggio
                   
                    bR = read(fdFIFO, message, sizeof(Message)); // Legge il messaggio dalla fifo
                    // vari controlli della lettura
                    if (bR == -1){
                        #ifdef DEBUG
                        printf("<PID %i> La FIFO potrebbe essere danneggiata\n", getpid());
                        #endif
                        ErrExit("read failed");
                    }
                    if (bR != sizeof(Message) || bR == 0){
                        #ifdef DEBUG
                        printf("<PID %i> I messaggi da leggere sono finiti\n", getpid());
                        #endif
                    } else { 
                        #ifdef DEBUG
                        printf("<PID %i> Messaggio consegnato\n", getpid());
                        #endif
                        // Se tutto è stato letto correttamente
                          
                        
                        #ifdef DEBUG
                        printf("\n");
                        #endif
                       

                        // Scrivi sulla lista dei device
                       
                        current_pid_message->message.max_distance = message->max_distance;
                        current_pid_message->message.message_id = message->message_id;
                        current_pid_message->message.pid_receiver = message->pid_receiver;
                        current_pid_message->message.pid_sender = message->pid_sender;
                        current_pid_message->next = (Pid_message *)malloc(sizeof(Pid_message));
                        strcpy(current_pid_message->message.message, message->message);
                        
                        
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
                        semOp(semidAck, 0, 1); 
                    }

                } while(bR > 0);

                

                // Invio messaggio
                semOp(semidBoard, pid_i, -1); // entra il figlio i
                int current_x;
                int current_y;

                // cerco il la posizione del pid attuale
                for (int i = 0; i < BOARD_DIM; i++){
                    for(int j = 0; j < BOARD_DIM; j++){
                        if(Board->Board[i][j] == getpid()){
                            current_x = i; 
                            current_y = j; 
                        }
                    }
                }

                for (int i = 0; i < BOARD_DIM; i++){
                    for(int j = 0; j < BOARD_DIM; j++){
                        // Se trovo un device sulla board che non è questo Device 
                        // allora comincio a scorrere la lista dei messaggi del Device corrente
                        if(Board->Board[i][j] != getpid() && Board->Board[i][j] != 0){
                           
                           
                            // Inizia a scorrere la lista di messaggi
                            Pid_message * current = pid_message;
                            while (current->next != NULL){
                                // Controlla che il messaggio rientri nel raggio d'azione dato dalla max_dist del messaggio correntemente analizzato in devicelist
                                semOp(semidAck, 0, -1); 
                                if(message_deliverbale(current_x, current_y, i, j, current->message.max_distance) &&
                                   messageID_in_Acknowledgelist(current_pid_message->message.message_id, AcknowledgeList)){
                                    
                                    
                                    printf("<%i> Messagio spedibile \n", getpid());
                                   

                                    
                                    int AckLstIndex = 0;
                                    
                                    // Controlla che il messaggio non sia già stato ricevuto dal device selezionato per l'nvio
                                    // o che non sia stato già inviato dal medesimo device 
                                  
                                    while(AckLstIndex < ACK_LIST_DIM && 
                                            !(current->message.message_id == AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id &&
                                            (/*getpid() == AcknowledgeList -> Acknowledgment_List[AckLstIndex].pid_sender  ||*/
                                            Board->Board[i][j] == AcknowledgeList -> Acknowledgment_List[AckLstIndex].pid_receiver))){
                                        
                                        AckLstIndex++;
                                    }
                                    
                                    

                                    // Se tutte le condizioni sono sodisfatte
                                    if (AckLstIndex == ACK_LIST_DIM){
                                        send_message(Board->Board[i][j], current->message);
                                        
                                    }
                                    
                                }
                                current = current->next;
                                semOp(semidAck, 0, 1); 
            
                            }

                        }
                    }
                }
                

                // MOVIMENTO
                if(i != 0)
                    Board -> Board[x][y]= 0;
                if (Board -> Board[current->x][current->y] == 0){
                    // Se la posizione è libera allora scrivi il pid in tale poszione e aggiorna i valori di x e y
                    Board -> Board[current->x][current->y] = getpid();
                    x = current->x;
                    y = current->y;
                    i++;
                }

                
                // controllo che sial il 1 device per stampare la stringa iniziale
                if (pid[0] == 0)
                    printf("# Step %i: device positions ########################\n", step++);
              
                
                for (int i = 0; i < BOARD_DIM; i++)
                    for(int j = 0; j < BOARD_DIM; j++)
                        if(Board->Board[i][j] == getpid()){
                            printf("%i %i %i msgs: ", getpid(), i, j);
                            Pid_message * current = pid_message;
                            while (current->next != NULL){
                                printf("%i ", current->message.message_id);
                                current = current->next;
                            }
                            printf("\n");

                        }

                
                // Controllo di che sial il 5 device per stampare la stringa finale altrimenti apro il prossimo semaforo
                if (pid[0] != 0 && pid[1] != 0 && pid[2] != 0 && pid[3] != 0 && pid[4] == 0){
                    printf("#############################################\n\n");
                } else 
                    semOp(semidBoard, pid_i+1, 1); // libera il fgilio i + 1 


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


    pid_t pid_ackManager = fork();

    if (pid_ackManager == -1){
        ErrExit("Fork failed");
    }

    // Codice del Ack-Manager
    if (pid_ackManager == 0){

        while(1){
            AckManage ackManage[20] = {0};
            semOp(semidAck, 0, -1);
            int AckLstIndex = 0;
            while (AckLstIndex < ACK_LIST_DIM){
                if (AcknowledgeList -> Acknowledgment_List[AckLstIndex].timestamp != 0){
                    int counter = ackManage[AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id].counter++;

                    //printf("%i\n", counter);
                    ackManage[AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id].index[counter] = AckLstIndex;
                    
                    if (ackManage[AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id].counter == 5){
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
            semOp(semidAck, 0, 1); 
        }
    }

    int step = 0;

    // TEST
    Message this_message1;
    this_message1.pid_sender = getpid();
    this_message1.pid_receiver = pid[1];
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
    close(deviceFIFO);
/*
    // TEST 2
    this_message1.pid_sender = getpid();
    this_message1.pid_receiver = pid[2];
    this_message1.message_id = 1;
    strcpy( this_message1.message, "char");
    this_message1.max_distance = 1;
    sprintf(pid2string, "%d", this_message1.pid_receiver);
    strcat(path_FIFO, pid2string);
    deviceFIFO = open(path_FIFO, O_WRONLY);
    if(deviceFIFO == -1)
        ErrExit("Open FIFO failed");
    bW = write(deviceFIFO, &this_message1, sizeof(Message));
    if (bW == -1 || bW != sizeof(Message)){
        ErrExit("Write failed");
    }
    close(deviceFIFO);
    */

    while(1){
        sleep(PACE_TIMER);
        // Stampa info pid
        //printf("# Step %i: device positions ########################\n", step);
        for(int pid_i = 0; pid_i < 5; pid_i++)
            // ricerca del pid all'interno della Board
            /*for (int i = 0; i < BOARD_DIM; i++)
                for(int j = 0; j < BOARD_DIM; j++)
                    if(Board->Board[i][j] == pid[pid_i])
                        printf("%i %i %i msgs: \n", pid[pid_i], i, j);*/
        //printf("#############################################\n\n");
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

        semOp(semidAck, 0, -1);
        #ifdef VIEWACKLIST // stampa acknowledge list
        int AckLstIndex = 0;
        while (AckLstIndex < 20){
            printf("%i | %i | %i | %ld\n",AcknowledgeList -> Acknowledgment_List[AckLstIndex].pid_sender,
            AcknowledgeList -> Acknowledgment_List[AckLstIndex].pid_receiver, AcknowledgeList -> Acknowledgment_List[AckLstIndex].message_id ,
            AcknowledgeList -> Acknowledgment_List[AckLstIndex].timestamp);
            AckLstIndex++;

        }
        #endif
        semOp(semidAck, 0, 1);

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