#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include "fifo.h"
#include "defines.h"
#include "err_exit.h"
#include <unistd.h>

#define DEBUG


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

    return 0;
}