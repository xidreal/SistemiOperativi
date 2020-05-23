/// @file client.c
/// @brief Contiene l'implementazione del client.
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "defines.h"

int main(int argc, char * argv[]) {
    
    struct message this_message;
    // Richiesta informazioni
    // Pid sender
    this_message.pid_sender = getpid();

    // Pid receiver
    pid_t pid_receiver = 0;
    printf("Inserire il pid del ricevente: ");
    do {
        scanf("%d", &pid_receiver);
        if (pid_receiver <= 0)
            printf("Inserire un valore positivo");
    } while (pid_receiver <= 0);
    this_message.pid_receiver = pid_receiver;

    // message_id
    int message_id = 0;
    printf("Inserire il message_id del messaggio: ");
    do {
        scanf("%i", &message_id);
        if (message_id <= 0)
            printf("Inserire un valore positivo");
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
        scanf("%f", &max_dist);
        if (max_dist <= 0)
            printf("Inserire un valore positivo");
    } while (max_dist <= 0);
    this_message.message_id = message_id;

    // Stampa la struttura
    printf("message struct: \n pid_sender: %d \n pid_receiver: %d \n message_id: %i \n message: %s \n max_dist: %f \n", 
            this_message.pid_sender, this_message.pid_receiver, this_message.message_id,
            (char *) this_message.message, this_message.max_dist );

    printf("%s",  path2fifo(pid_receiver));
   
    return 0;
}