/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once
#include <unistd.h>

// Struttura messaggio
struct message {
    pid_t pid_sender;
    pid_t pid_receiver;
    int message_id;
    char message[256]; 
    float max_dist;
};

int prova2;