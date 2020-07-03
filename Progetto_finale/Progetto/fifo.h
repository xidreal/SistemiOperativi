/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.
#include <string.h>
#include <sys/types.h>
#include "defines.h"

#pragma once

char * path2fifo(pid_t pid);

// Iniva il messaggio ad un PID con un certo device
void send_message(pid_t pid, Message message);