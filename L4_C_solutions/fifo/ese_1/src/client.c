#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "errExit.h"
#include <string.h>


typedef struct test{
  int a;
  char b[3];
} test;


int main (int argc, char *argv[]) {
    // Checkg command line input arguments
    // The program only wants a FIFO pathname
    if (argc != 2) {
        printf("Usage: %s fifo_pathname\n", argv[0]);
        return 0;
    }
    
    // read the FIFO's pathname
    char *path2ServerFIFO = argv[1];

    printf("<Client> opening FIFO %s...\n", path2ServerFIFO);
    // Open the FIFO in write-only mode
    int serverFIFO = open(path2ServerFIFO, O_WRONLY);
    if(serverFIFO == -1)
        errExit("open fifo failed");

    int v [] = {0, 0};
    printf("<Client> Give me two numbers: ");
    scanf("%d %d", &v[0], &v[1]);
   
    test prova;
    prova.a = 1;
    prova.b[0] = "ab";
    strcpy(prova.b, "ab");
    
    printf("<Client> sending %d %d\n", v[0], v[1]);
    // Wrinte  two integers to the opened FIFO
    if(write(serverFIFO, &prova, sizeof(test)) != sizeof(test))
        errExit("write failed");

    // Close the FIFO
    if(close(serverFIFO) != 0)
        errExit("close failed");
}
