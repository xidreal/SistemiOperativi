/// @file server_lib.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del server.

#include "server.h"
#include <sys/unistd.h>

//#define DEBUG // Abilita le stampe di debug.

void test_process(int pid_i, int message_id, int sec);

void sorting_date(AckMessage * ackMessage);