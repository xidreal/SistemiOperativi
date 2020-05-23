/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include "err_exit.h"
#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * path2fifo(pid_t pid){
    char pid2char[10];
    itoa(pid, pid2char, 10);
    char path[sizeof(FIFOPATH) + sizeof(pid2char)] = FIFOPATH;
    strcat(path, pid2char);
    return path;
}