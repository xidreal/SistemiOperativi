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
//#define VERBOSE //attiva le stampe di DEBUG

int main(int argc, char * argv[]) {
    
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
        if (message_id <= 0)
            printf("Inserire un valore positivo: ");
    } while (message_id <= 0);
    this_message.message_id = message_id;

    // messaggio
    printf("Inserire il testo del messaggio: ");
    scanf("%s", this_message.message);
    //fgets(this_message.message, sizeof(this_message.message), stdin);
    //this_message.message[strlen(this_message.message)-1] = '\0';

    // message_id
    float max_dist = 0;
    printf("Inserire la distanza di invio del messaggio: ");
    do {
        scanf("%d", &max_dist);
        getchar();
        if (max_dist <= 0)
            printf("Inserire un valore positivo: ");
    } while (max_dist <= 0);
    this_message.message_id = message_id;

    // DEBUG: Stampa la struttura del messaggio
    #ifdef VERBOSE
    printf("message struct: \n pid_sender: %d \n pid_receiver: %d \n message_id: %i \n message: %s \n max_dist: %d \n", 
            this_message.pid_sender, this_message.pid_receiver, this_message.message_id,
            (char *) this_message.message, this_message.max_dist );
    #endif
   
    // Crea la path della FIFO del device
    char path_FIFO[15+10] = "/tmp/dev/_fifo.";
    char pid2string[10];
    sprintf(pid2string, "%d", pid_receiver);
    strcat(path_FIFO, pid2string);

    // DEBUG: Stampa la path della FIFO del device
    #ifdef VERBOSE
    printf("%s", path_FIFO);
    #endif

    int deviceFIFO = open(path_FIFO, O_WRONLY);
    if(deviceFIFO == -1)
        ErrExit("Open FIFO failed");

    return 0;
}