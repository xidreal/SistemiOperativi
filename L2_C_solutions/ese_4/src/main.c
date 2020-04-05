#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>

#include "errExit.h"

#define MAX_NUM 100

int main (int argc, char *argv[]) {

    // init. seed for rand function
    srand(time(0));

    // generate two random numbers
    int n = (int)(((double)rand() / RAND_MAX) * MAX_NUM);
    int m = (int)(((double)rand() / RAND_MAX) * MAX_NUM);

    //....

    // call execl to run moltiplicatore
    execl("moltiplicatore", /*...*/);
    errExit("execlp failed");

    return 1;
}
