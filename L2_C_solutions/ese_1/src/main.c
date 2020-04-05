#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "errExit.h"

#define BUFFER_SZ 150

int main (int argc, char *argv[]) {

    // Get the username of user
    char *username = //....
    if (username == NULL)
        username = "unknown";

    // Get the home directory of the user
    char *homeDir  = //....
    if (homeDir == NULL) {
        printf("unknown home directory\n");
        exit(0);
    }

    // Get the current process's working directory
    char buffer[BUFFER_SZ];
    //....

    // check if the current process's working directory is a sub directory of
    // the user's home directory
    int subDir = //....

    if (subDir == 0)
        printf("Caro %s, sono gia' nel posto giusto!\n", username);
    else {
        // move the process into the user's home directory
        //....

        // create an empty file. DO NOT CHANGE THE PATHNAME in the open system call!
        int fd = open("empty_file.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1)
            errExit("open failed");

        // close the file descriptor of the empty file
        //...

        printf("Caro %s, sono dentro la tua home!\n", username);
    }

    return 0;
}
