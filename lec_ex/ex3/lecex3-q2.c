#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void * copy_file( void * arg ); // thread function prototype

int main (int argc, char** argv) {
    pthread_t tid[argc - 1];
    int total_bytes_copied = 0;
    int* bytes_copied;
    int rc;

    for (int i = 1; i < argc; i++) {
        printf("MAIN: Creating thread to copy \"%s\"\n", *(argv + i));
        rc = pthread_create(&tid[i - 1], NULL, copy_file, (void *) *(argv + i));
        if (rc != 0) {
            fprintf(stderr, "ERROR: pthread_create() failed...\n");
            return EXIT_FAILURE;
        }
    }

    for (int i = 1; i < argc; i++) {
        rc = pthread_join(tid[i - 1], (void **)&bytes_copied);
        if (rc != 0) {
            fprintf(stderr, "ERROR: pthread_join() failed...\n");
            return EXIT_FAILURE;
        }
        if (*bytes_copied == 1) {
            printf("MAIN: Thread completed copying %d byte for \"%s\"\n", *bytes_copied, *(argv + i));
        } else {
            printf("MAIN: Thread completed copying %d bytes for \"%s\"\n", *bytes_copied, *(argv + i));
        }
        total_bytes_copied += *bytes_copied;
        free(bytes_copied);
    }

    if (argc == 2) {
        if (total_bytes_copied == 1) {
            printf("MAIN: Successfully copied %d byte via %d child thread\n", total_bytes_copied, argc - 1);
        } else {
            printf("MAIN: Successfully copied %d bytes via %d child thread\n", total_bytes_copied, argc - 1);
        }
    } else {
        if (total_bytes_copied == 1) {
            printf("MAIN: Successfully copied %d byte via %d child threads\n", total_bytes_copied, argc - 1);
        } else {
            printf("MAIN: Successfully copied %d bytes via %d child threads\n", total_bytes_copied, argc - 1);
        }
    }

    return EXIT_SUCCESS;
}