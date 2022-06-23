/* hw1.c
 *  Author: Martin Smith
 *  CSCI 4210
 *  06/08/22
 *  Goldschmidt
 * 
 *  Submitty score: 50/50
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define BUFFER_SIZE 128

int hash(char* c, int cache) {
    int sum = 0;
    for (int i = 0; i < strlen(c); i++)
    {
        sum += *(c + i);
    }
#ifdef DEBUG_MODE
    printf("Sum of ASCII values: %d\n", sum);
#endif
    return (sum % cache);
}

int main(int argc, char** argv) {
    setvbuf( stdout, NULL, _IONBF, 0 );

    if (argc < 3) {
        fprintf(stderr, "ERROR: Incorrect number of arguments provided!\n");
        return EXIT_FAILURE;
    }

    int cache_size = atoi(*(argv + 1));
    if (cache_size == 0) {
        fprintf(stderr, "ERROR: Invalid cache size!\n");
        return EXIT_FAILURE;
    }

    char** cache_table = calloc(cache_size, sizeof(char*));
    char* chunk = calloc(BUFFER_SIZE, sizeof(char)); // Temporary character buffer array used to store read words
    char* word = calloc(BUFFER_SIZE, sizeof(char)); // Character array that will store individual words

    
    char *file = *(argv + 2);
    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "ERROR: open() failed!\n");
        return EXIT_FAILURE;
    }

    int rd, wrd_len, hash_slot, word_begin = -1;
    /* -1 is the default value given to word_begin, meaning that it refers to nothing */

    do
    {
        rd = read(fd, chunk, BUFFER_SIZE - 1);
        *(chunk + rd) = '\0';
    #ifdef DEBUG_MODE
        printf("Number of bytes read: %d\n", rd);
        printf("chunk: %s\n", chunk);
    #endif
        
        for (int j = 0; j < strlen(chunk); j++) {
            if ((isalnum(*(chunk + j)) != 0) && word_begin == -1) { // Valid first character found
                word_begin = j;
            } else if ((isalnum(*(chunk + j)) == 0) && word_begin != -1) { // Delimiter found, perform logic to determine if valid word is formed
                if ((j - word_begin) > 1) { // Valid word found, hash here and store
                    wrd_len = j - word_begin;
                    word = strncpy(word, (chunk + word_begin), wrd_len);
                    *(word + wrd_len) = '\0';
                    hash_slot = hash(word, cache_size);
                #ifdef DEBUG_MODE
                    printf("Parsed word: %s\n", word);
                    printf("Hash slot: %d\n", hash_slot);
                #endif
                    if (*(cache_table + hash_slot) == NULL) { // Allocate new memory to store word
                        *(cache_table + hash_slot) = calloc(wrd_len + 1, sizeof(char));
                        strcpy(*(cache_table + hash_slot), word);
                        printf("Word \"%s\" ==> %d (calloc)\n", word, hash_slot);
                    } else {
                        if (strlen(*(cache_table + hash_slot)) != wrd_len) { // Replacement word has a different size compared to original
                            *(cache_table + hash_slot) = realloc(*(cache_table + hash_slot), wrd_len + 1);
                            strcpy(*(cache_table + hash_slot), word);
                            printf("Word \"%s\" ==> %d (realloc)\n", word, hash_slot);
                        } else { // Old and new words have the same size
                            strcpy(*(cache_table + hash_slot), word);
                            printf("Word \"%s\" ==> %d (nop)\n", word, hash_slot);
                        }
                    }
                }
                word_begin = -1;
            }

            if (j == (strlen(chunk) - 1) && word_begin != -1) { // Potential word in-between chunks
                int seek = lseek(fd, -(j - word_begin + 1), SEEK_CUR);
                if (seek == -1) {
                    fprintf(stderr, "ERROR: lseek() failed!\n");
                    return EXIT_FAILURE;
                }
            #ifdef DEBUG_MODE
                printf("Offset location moved: %d\n", seek);
            #endif
                word_begin = -1;
            }

        }

    } while (rd == BUFFER_SIZE - 1);
        
    for (int i = 0; i < cache_size; i++) {
        if (*(cache_table + i) != NULL)
        {
            printf("Cache index %d ==> \"%s\"\n", i, *(cache_table + i));
            free(*(cache_table + i));
        }
    }

    free(cache_table);
    free(chunk);
    free(word);    

    return EXIT_SUCCESS;
}