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
    
    
}