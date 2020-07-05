/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.


#pragma once

#include <sys/types.h>  // Tipi di variabile

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

// Struttura dati di supporto dell'ackmanger
typedef struct{
    int counter;
    int index[5];
} AckManage;

typedef struct {
    long mtype;
    Acknowledgment acks [5];
} AckMessage;


// Trasfora il file passato in liste di posizioni per ogni device
void file_to_list(Position * position_pid[], int file);

// Controlla che due punti siano sotto una certa distanza
int message_deliverbale(int x, int y, int i, int j, int distance);

void print_list(Pid_message * head);

void test_process(int pid_i, int message_id, int sec);

