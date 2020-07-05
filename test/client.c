#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include "fifo.h"
#include "defines.h"
#include "err_exit.h"
#include <unistd.h>
#include <sys/msg.h>
#include <sys/stat.h>

#define DEBUG

typedef struct{
    long mtype;
    int test;
} AckMessage;


int main(int argc, char * argv[]) {
    
    int message_id = 2;
    char path_output[14] = "out_";
    char messageid2string[6];
    sprintf(messageid2string, "%d", message_id);
    strcat(path_output, messageid2string);
    strcat(path_output, ".txt");
    printf("sto creando il file\n");
    int output_fd = open(path_FIFO, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR, S_IWUSR);
    if(output_fd==-1)
        ErrExit("open failed");

    
    char * buffer = "prova";
 
    write(output_fd, buffer, sizeof(buffer));
}