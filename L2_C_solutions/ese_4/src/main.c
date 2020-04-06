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

    char nStr [100];
    char mStr [100];

    sprintf(nStr, "%d", n);
    sprintf(mStr, "%d", m);

    // call execl to run moltiplicatore
    execl("moltiplicatore", "moltiplcatore", nStr, mStr, (char *) NULL);
    errExit("execlp failed");

    return 1;
}
