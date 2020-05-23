/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.
#include <unistd.h>

#pragma once

#define FIFOPATH "/tmp/dev/_fifo."
// Crea la path alla fifo
char * path2fifo(pid_t pid);