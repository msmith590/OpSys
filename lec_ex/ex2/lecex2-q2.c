/* lecex2-q2.c */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


int lecex2_child( int n ) {
    int fd = open("lecex2.txt", O_RDONLY);
    if (fd == -1) {
        perror("open() failed");
        abort();
    }
    char* buf = calloc(16, sizeof(char));
    int bytes_read = 0, total_bytes = 0;
    char c;

    if (n <= 16) {
        bytes_read = read(fd, buf, 16);
        if (bytes_read < n) {
            fprintf(stderr, "%dth character does not exist!\n", n);
            close(fd);
            free(buf);
            abort();
        }
    } else {
        while (n - total_bytes > 0) {
            bytes_read = read(fd, buf, 16);
            total_bytes += bytes_read;
            if (bytes_read == 0 && n != total_bytes) {
                fprintf(stderr, "%dth character does not exist!\n", n);
                close(fd);
                free(buf);
                abort();
            }
        }
    }

    close(fd);
    c = *(buf + ((n - 1) % 16));
    free(buf);
    return c;
}

int lecex2_parent() {
    int status;
    waitpid(-1, &status, 0);

    if (WIFSIGNALED(status)) {
        printf("PARENT: whoops, child process terminated abnormally!\n");
        return EXIT_FAILURE;
    } else if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        printf("PARENT: child process successfully returned '%c'\n", exit_status);
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}