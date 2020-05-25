/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include <stdio.h>

#define DEBUG
//#define VERBOSE


int main(int argc, char * argv[]) {
    
    // Crea la memoria condivisa per ospitare la Board
    int shmidBoard = alloc_shared_memory(IPC_PRIVATE, (sizeof(pid_t) * BOARD_DIM * BOARD_DIM) + sizeof(key_t));
    SharedBoard * Board = (SharedBoard *)get_shared_memory(shmidBoard, 0);
    Board -> Board[1][1]= 2;
    
    // DEBUG: shared memory
    #ifdef DEBUG
    pid_t pid = fork();
    if(pid == 0){
        //int shmidBoard = alloc_shared_memory(IPC_PRIVATE, (sizeof(pid_t) * BOARD_DIM) + sizeof(key_t));
        SharedBoard * Board= (SharedBoard *)get_shared_memory(shmidBoard, 0);
        Board ->Board[1][1] = 4;
        printf("%d \n", Board -> Board[1][1]);
    }
    
    printf("%d \n", Board -> Board[1][1]);
    #endif

    // Detach del segmento
    free_shared_memory(Board);
    // Rimozione della memoria condivisa
    remove_shared_memory(shmidBoard);

    /*/ Scacchiera
    Board = pid_t [10][10];
    Key Board_memory = Genera segmento di memoria Board;
    Crea semaforo SEM_IDX_BOARD(0,0,0,0,0);
    // Acknowledge_list
    Crea semaforo SEM_IDX_ACK;
    Key Acknowledge_memory = Genera segmento di memoria Acknowlodge_list[100];
    pid_t Device[5];
    // Apri file posizioni
    Apri file_posizioni;*/

    return 0;
}