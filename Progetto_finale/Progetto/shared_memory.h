/// @file shared_memory.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#pragma once

#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "defines.h"

#define BOARD_DIM 10
#define ACK_LIST_DIM 100

typedef struct{
    key_t key;
    pid_t Board [BOARD_DIM][BOARD_DIM];
} SharedBoard;

typedef struct{
    key_t key;
    Acknowledgment Acknowledgment_List [ACK_LIST_DIM];
} AckList;


// The alloc_shared_memory method creates, if it does not exist, a shared
// memory segment with size bytes and shmKey key.
// It returns the shmid on success, otherwise it terminates the calling process
int alloc_shared_memory(key_t shmKey, size_t size);

// The get_shared_memory attaches a shared memory segment in the logic address space
// of the calling process.
// It returns a pointer to the attached shared memory segment,
// otherwise it terminates the calling process
void *get_shared_memory(int shmid, int shmflg);

// The free_shared_memory detaches a shared memory segment from the logic
// address space of the calling process.
// If it does not succeed, it terminates the calling process
void free_shared_memory(void *ptr_sh);

// The remove_shared_memory removes a shared memory segment
// If it does not succeed, it terminates the calling process
void remove_shared_memory(int shmid);

// Controlla se nella memoria condivisa è presente un message con message_id specificato
int messageID_in_Acknowledgelist(int message_id, AckList * AcknowledgeList);
