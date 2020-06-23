/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once
#include <unistd.h>
#include <time.h>

#define BUFFER_SZ 20

typedef struct Position{
    int x;
    int y;
    struct Position *next;
} Position;

typedef struct{
    Position *next;
} Position_head;

typedef struct {
    pid_t pid_sender;
    pid_t pid_receiver;
    int message_id;
    char message[256];
    double max_distance;
} Message;

typedef struct {
    pid_t pid_sender;
    pid_t pid_receiver;
    int message_id;
    time_t timestamp;
} Acknowledgment;