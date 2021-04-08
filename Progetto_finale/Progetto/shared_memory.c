/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include "err_exit.h"
#include "shared_memory.h"
#include <stdio.h>

int alloc_shared_memory(key_t shmKey, size_t size) {
    // get, or create, a shared memory segment
    int mem;
    if((mem = shmget(shmKey, size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1)
        ErrExit("shmget failed");

    return mem;
}

void *get_shared_memory(int shmid, int shmflg) {
    // attach the shared memory
    void *attach;
    if ((attach = (void *) shmat(shmid, NULL, shmflg)) == (void *) -1)
        ErrExit("attach failed");

    return attach;
}

void free_shared_memory(void *ptr_sh) {
    // detach the shared memory segments
    if(shmdt(ptr_sh) == -1)
        ErrExit("shmdt failed");
}

void remove_shared_memory(int shmid) {
    // delete the shared memory segment
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
        ErrExit("shmctl remove failed");
}

int messageID_in_Acknowledgelist(int message_id, AckList * AcknowledgeList ){
    int i = 0;
    while(AcknowledgeList -> Acknowledgment_List[i].message_id != message_id && i < ACK_LIST_DIM) // Trova la riga di un determinato message id
        i++;
    //printf("%i == %i, %i \n", AcknowledgeList -> Acknowledgment_List[i].message_id, message_id, i);
    
    if ((AcknowledgeList -> Acknowledgment_List[i].timestamp == 0 && message_id != 0 )|| i == ACK_LIST_DIM || message_id == 0){
        //printf("Messaggio scaduto, message_id = %i\n", message_id);
        return 0; // il message_id non è disponibile in acklist
    } else{ 
        //printf("Messaggio non cancellabile, message_id = %i\n", message_id);
        return 1; // Il message_id è acora disponiobile
    }
    
}