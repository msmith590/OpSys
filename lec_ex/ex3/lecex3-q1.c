#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <ctype.h>

/**
 * @brief Function that capitalizes all lowercase letters in a shared memory segment
 * 
 * @param pipefd 
 * @return int 
 */
int lecex3_q1_child( int pipefd ) {
    int key, size;
    if (read(pipefd, &key, 4) != 4) {
        fprintf(stderr, "ERROR: Problem reading in shared memory key...\n");
        return EXIT_FAILURE;
    }
    if (read(pipefd, &size, 4) != 4) {
        fprintf(stderr, "ERROR: Problem reading in size of shared memory segment...\n");
        return EXIT_FAILURE;
    }

    int shmid = shmget( key, sizeof( int ), 0 );

    char* data = shmat(shmid, NULL, 0); // child attaches to shared memory segment
    if (data == (void *) -1) {
        perror("ERROR");
    }
    for (int i = 0; i < size; i++) {
        *(data + i) = toupper(*(data + i));
    }
    int rc = shmdt(data);
    if (rc == -1) {
        perror("ERROR");
    }

    return EXIT_SUCCESS;
}