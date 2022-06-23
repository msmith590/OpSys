#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 129

int main(int argc, char** argv) {
    setvbuf( stdout, NULL, _IONBF, 0 );

    if (argc < 2) {
        fprintf(stderr, "ERROR: Must provide at least 1 file to parse...\n");
        return EXIT_FAILURE;
    }

    int* pipefd = calloc(2, sizeof(int));
    if (pipe(pipefd) == -1) {
        perror("ERROR");
        return EXIT_FAILURE;
    } else {
        printf("PARENT: Created pipe successfully\n");
    }

#ifdef DEBUG_MODE
    printf("pipefd[0]: %d\n", *(pipefd));
    printf("pipefd[1]: %d\n", *(pipefd + 1));
#endif

    /* TODO: Parent must fork and create a child here to execute hidden file */

    int fileno = 1;     /* Counter that tracks which file child needs to parse */

    int fd = open(*(argv + fileno), O_RDONLY);
    if (fd == -1) {
        perror("ERROR");
        exit(2);
    }

    char* buffer = calloc(BUFFER_SIZE, sizeof(char));
    char* word = calloc(BUFFER_SIZE, sizeof(char));
    int bytes_read = 0, words_written = 0, word_len = 0;
    do
    {
        bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read == -1) {
            perror("ERROR");
            exit(1);
        }
        *(buffer + bytes_read) = '\0';
    #ifdef DEBUG_MODE
        printf("\nRead the chunk \'%s\' (%d byte(s)) from %s\n\n", buffer, bytes_read, *(argv + fileno));
    #endif
        int word_begin = -1;
        for(int i = 0; i < bytes_read; i++) {
            if (word_begin == -1 && isalnum(*(buffer + i))) { /* Valid first character found */
                word_begin = i;
            } else if (word_begin != -1 && !isalnum(*(buffer + i))) { /* Delimiter found */
                word_len = i - word_begin;
                if (word_len < 2) { /* Invalid word, reset word_begin index */
                    word_begin = -1;
                } else { /* Valid word, packaging to write to pipe */
                    word = strncpy(word, (buffer + word_begin), word_len);
                    *(word + word_len) = '.';
                    *(word + word_len + 1) = '\0'; /* Safety measure, will not be written into pipe */
                #ifdef DEBUG_MODE
                    printf("Parsed word with \'.\' added: \'%s\'\n", word);
                #endif
                    if (write(*(pipefd + 1), word, word_len + 1) == -1) {
                        perror("ERROR");
                        exit(1);
                    } else {
                        words_written++;
                    #ifdef DEBUG_MODE
                        printf("Successfully wrote \'%s\' to pipe\n", word);
                    #endif
                    }
                    word_begin = -1;
                }
            } else if (i == bytes_read - 1 && isalnum(*(buffer + i)) && bytes_read < BUFFER_SIZE - 1) { 
                /* Handles corner case of a missing delimiter at end of file */
            #ifdef DEBUG_MODE
                printf("Corner case: missing delimiter at end of file\n");
            #endif
                word_len = i - word_begin + 1;
                if (word_len < 2) { /* Invalid word, reset word_begin index */
                    word_begin = -1;
                } else { /* Valid word, packaging to write to pipe */
                    word = strncpy(word, (buffer + word_begin), word_len);
                    *(word + word_len) = '.';
                    *(word + word_len + 1) = '\0'; /* Safety measure, will not be written into pipe */
                #ifdef DEBUG_MODE
                    printf("Parsed word with \'.\' added: \'%s\'\n", word);
                #endif
                    if (write(*(pipefd + 1), word, word_len + 1) == -1) {
                        perror("ERROR");
                        exit(1);
                    } else {
                        words_written++;
                    #ifdef DEBUG_MODE
                        printf("Successfully wrote \'%s\' to pipe\n", word);
                    #endif
                    }
                    word_begin = -1;
                }
            } else if (i == bytes_read - 1 && isalnum(*(buffer + i)) && word_begin != -1) {
                int seek = lseek(fd, -(i - word_begin + 1), SEEK_CUR);
                if (seek == -1) {
                    perror("ERROR");
                    exit(1);
                }
            #ifdef DEBUG_MODE
                printf("Offset location moved: %d\n", seek);
            #endif
            }
        }
    } while (bytes_read == BUFFER_SIZE - 1);

    close(*(pipefd + 1)); /* Close write end of pipe in child */
    close(fd); /* Close read-only file in child */
    free(buffer);
    free(word);

    if (words_written > 1) {
        printf("CHILD: Successfully wrote %d words on the pipe\n", words_written);
        exit(0);
    } else if (words_written == 1) {
        printf("CHILD: Successfully wrote %d word on the pipe\n", words_written);
        exit(0);
    } else {
        printf("CHILD: Did not write any words on the pipe\n");
        exit(3);
    }
    
    close(*(pipefd + 1)); /* Close unused write end of pipe in parent */

    

    /* TODO: Parent must call waitpid here */

    /* TODO: Parent must create a child process that calls execl here */


    return EXIT_SUCCESS;
}