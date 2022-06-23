#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int hash(char* c, int cache) {
    int sum = 0;
    for (int i = 0; i < strlen(c); i++)
    {
        printf("ASCII value of %c: %d\n", *(c + i), (int) *(c + i));
        sum += *(c + i);
    }
    printf("Sum of ASCII values: %d\n", sum);
    return (sum % cache);
}

int main(int argc, char** argv) {
    char* test = malloc(20);
    char* half = malloc(20);
    strcpy(test, "Hello World!\n");
    strncpy(half, (test + 6), 5);
    *(half + 5) = '\0';

    printf("String in half: %s\n", half);

    printf("isalnum of null terminating byte: %d\n", isalnum('\0'));


    return EXIT_SUCCESS;
}