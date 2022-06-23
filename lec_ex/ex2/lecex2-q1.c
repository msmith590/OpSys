/* lecex2-q1.c */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int main () {
    printf("PARENT: start here.\n");
    fflush(stdout); // flushes buffer and prevents child from outputing duplicate string above

    pid_t pid;
    int status;
    pid = fork();

    if (pid == -1) {
        perror("fork() failed");
        return EXIT_FAILURE;
    }
    
    if (pid == 0) { // Child process executes here
        printf("CHILD: happy birthday to me!\n");
        printf("CHILD: ok......good-bye.\n");
        return EXIT_SUCCESS;
    } else if (pid > 0) { // Parent process executes here
        waitpid(pid, &status, 0); /* Blocking until child terminates*/

        if (WIFSIGNALED(status)) {
            fprintf(stderr, "child terminated abnormally\n");
        } else if (WIFEXITED(status)) {
            printf("PARENT: i'm still here.\n");
        }
    }


    return EXIT_SUCCESS;
}