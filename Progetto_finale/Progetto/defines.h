/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.


#pragma once

#include <sys/types.h>  // Tipi di variabile

//#define DEBUG
#define PACE_TIMER 2 // Tempo di esecuzione degli spostamenti in secondi
#define BUFFER_SZ 21
#define DEBUG

typedef struct Position{
    int x;
    int y;
    struct Position *next;
} Position;

typedef struct {
    pid_t pid_sender;
    pid_t pid_receiver;
    int message_id;
    char message[256];
    double max_distance;
} Message;

typedef struct Pid_message{
    Message message;
    struct Pid_message *next;
} Pid_message;

typedef struct {
    pid_t pid_sender;
    pid_t pid_receiver;
    int message_id;
    time_t timestamp;
} Acknowledgment;

// Trasfora il file passato in liste di posizioni per ogni device
void file_to_list(Position * position_pid[], int file);