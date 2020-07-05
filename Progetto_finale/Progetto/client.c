/// @file client.c
/// @brief Contiene l'implementazione del client.
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include "fifo.h"
#include "defines.h"
#include "err_exit.h"
#include <unistd.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/msg.h>


#define DEBUG //attiva le stampe di DEBUG

int main(int argc, char * argv[]) {
    
    if (argc != 2){
        printf("Usage: %s msg_queue_key file_posizioni\n", argv[0]);
        ErrExit("Incorrect args value");
    }

    // Creo la coda di messaggi
    int msqid;
    key_t msg_queue_key = atoi(argv[1]);
    if((msqid = msgget(msg_queue_key, S_IRUSR | S_IWUSR)) ==-1)
        ErrExit("msgget failed");
    printf("MSQID %i ---------------------------------------------\n", msqid);

    Message this_message;
    // Richiesta informazioni
    // Pid sender
    this_message.pid_sender = getpid();

    // Pid receiver
    pid_t pid_receiver = 0;
    printf("Inserire il pid del ricevente: ");
    do {
        scanf(" %d", &pid_receiver);
        getchar();
        if (pid_receiver <= 0)
            printf("Inserire un valore positivo: ");
    } while (pid_receiver <= 0);
    this_message.pid_receiver = pid_receiver;

    // message_id
    int message_id = 0;
    printf("Inserire il message_id del messaggio: ");
    do {
        scanf("%i", &message_id);
        getchar();
        if (message_id <= 0 || message_id > 20)
            printf("Inserire un valore compreso tra 1 e 20: ");
    } while (message_id <= 0);
    this_message.message_id = message_id;

    // messaggio
    printf("Inserire il testo del messaggio: ");
    scanf("%s", this_message.message);

    // message_id
    float max_dist = 0;
    printf("Inserire la distanza di invio del messaggio: ");
    do {
        scanf("%f", &max_dist);
        getchar();
        if (max_dist <= 0)
            printf("Inserire un valore positivo: ");
    } while (max_dist <= 0);
    this_message.max_distance = max_dist;

    // DEBUG: Stampa la struttura del messaggio
    #ifdef DEBUG
    printf("message struct: \n pid_sender: %d \n pid_receiver: %d \n message_id: %i \n message: %s \n max_dist: %f \n", 
            this_message.pid_sender, this_message.pid_receiver, this_message.message_id,
            (char *) this_message.message, this_message.max_distance );
    #endif
   
    // Crea la path della FIFO del device
    char path_FIFO[15+10] = "/tmp/dev_fifo.";
    char pid2string[10];
    sprintf(pid2string, "%d", pid_receiver);
    strcat(path_FIFO, pid2string);

    // DEBUG: Stampa la path della FIFO del device
    #ifdef DEBUG
    printf("%s", path_FIFO);
    #endif

    int deviceFIFO = open(path_FIFO, O_WRONLY);
    if(deviceFIFO == -1)
        ErrExit("Open FIFO failed");
    
    // write su FIFO
    int bW = write(deviceFIFO, &this_message, sizeof(Message));
    if (bW == -1 || bW != sizeof(Message)){
        #ifdef DEBUG
        printf("<PID %i> La FIFO potrebbe essere danneggiata\n", getpid());
        #endif
        ErrExit("Write failed");
    }

    close(deviceFIFO);

    AckMessage * ackMessage = (AckMessage *)malloc(sizeof(AckMessage));
    //size_t mSize = sizeof(ackMessage) - sizeof(ackMessage->mtype); 
    long mtype = message_id;

    printf("Attendo ack %ld\n", mtype);

    int bR = msgrcv(msqid, ackMessage, sizeof(AckMessage), message_id, 0);
    if(bR == -1)
        ErrExit("msgsnd failed");
    if(bR == 0)
        printf("Read from queue failed.\n");

    printf("messaggio: %i | %ld\n", ackMessage->acks[0].message_id, ackMessage->acks[0].timestamp);

     // Crea la path della FIFO del device
    char path_output[14] = "out_";
    char messageid2string[6];
    sprintf(messageid2string, "%d", message_id);
    strcat(path_output, messageid2string);
    strcat(path_output, ".txt");
    printf("sto creando il file\n");
    int output_fd = open(path_output, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR, S_IWUSR);
    if(output_fd==-1)
        ErrExit("open failed");


    char * buffer = "prova";
 
    write(output_fd, buffer, sizeof(buffer));
    
    return 0;
}