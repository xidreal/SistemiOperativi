/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once
#include <unistd.h>     //
#include <stdio.h>      // print
#include <time.h>       // Timestamp
#include <sys/msg.h>    // Msg_queue
#include <sys/types.h>  // Tipi di variabile
#include <sys/stat.h>   // Flag
#include <fcntl.h>      // Flag
#include <stdlib.h>     // Malloc.

//#define DEBUG
#define PACE_TIMER 2 // Tempo di esecuzione degli spostamenti in secondi
#define BUFFER_SZ 21

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

int control_IDMessage_in_Acknowledgelist(int message_id, AckList * AcknowledgeList);
