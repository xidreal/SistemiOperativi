/// @file server.h
/// @brief Contiene la definizioni di variabili globali
///         del server.

#pragma once

#include <sys/unistd.h>
#include "defines.h"
#include "shared_memory.h"

#define SEM_ACK 5
#define SEM_BOARD 6

#define REPEATPOSITION // Ripete le posizioni dei device invece di fermarsi sull'ultima

pid_t dev_pid[5];
int semid;
int msqid;
AckList *AcknowledgeList;
SharedBoard *Board;
Position *position_pid[5];
Position * current_pos[5];