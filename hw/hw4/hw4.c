#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXBUFFER 1024
#define MAX_CLIENTS 5

void capitalize(char* word) {
    for (int i = 0; i < strlen(word); i++) {
        word[i] = toupper(word[i]);
    }
}

int isSame(char* w1, char* w2) {
    char* temp1 = calloc(strlen(w1) + 1, sizeof(char));
    strncpy(temp1, w1, strlen(w1) + 1);
    char* temp2 = calloc(strlen(w2) + 1, sizeof(char));
    strncpy(temp2, w2, strlen(w2) + 1);

    capitalize(temp1);
    capitalize(temp2);
    int rc = 0;
    if (strcmp(temp1, temp2) == 0) { // words are the same
        rc = 1;
    }

    free(temp1);
    free(temp2);
    return rc;
}

char** parseDict(int dictfd, int longest_word, int* dictionary_size) {
    int num_words = 0;
    char letter;
    while (read(dictfd, &letter, 1)) {
        if (letter == '\n') {
            num_words++;
        }
    }
    lseek(dictfd, -1, SEEK_CUR);
    read(dictfd, &letter, 1);
    if (letter != '\n') {
        // Corner case: Last word does not have a newline
        num_words++;
    }
    lseek(dictfd, 0, SEEK_SET); // reset offset to beginning of file
    *dictionary_size = num_words;

    #ifdef DEBUG_MODE
    printf("Number of words in dictionary: %d\n", num_words);
    #endif

    char** dict = calloc(num_words, sizeof(char*));
    char* buffer = calloc(longest_word, sizeof(char));
    int word_length = 0;
    int bytes_read;
    for (int i = 0; i < num_words; i++) {
        do
        {
            bytes_read = read(dictfd, &letter, 1);
            if (bytes_read && letter != '\n') {
                buffer[word_length] = letter;
                word_length++;
            }
        } while (bytes_read && letter != '\n');

        dict[i] = calloc(word_length + 1, sizeof(char));
        memcpy(dict[i], buffer, word_length);
        dict[i][word_length] = '\0'; // redundant, but good to ensure string is null-terminated
        word_length = 0;
    }
    free(buffer);
    close(dictfd);

    #ifdef DEBUG_MODE
    printf("Printing contents of dictionary:\n");
    for (int i = 0; i < num_words; i++) {
        printf("%s\n", dict[i]);
    }
    #endif
    
    return dict;
}

/**
 * @brief Checks if name is a valid username
 * 
 * @param name 
 * @param usernames 
 * @return 0 if name is invalid; 1 if name is valid
 */
int checkUsernames(char* name, char** usernames) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (usernames[i] != NULL && isSame(name, usernames[i])) {
            return 0;
        }
    }
    return 1;
}

void compareWords(char* guess, char* target, int* correct_letters, int* correct_placement) {
    if (strlen(guess) != strlen(target)) {
        fprintf(stderr, "ERROR: Comparing words of different length!\n");
        abort();
    }
    char* temp1 = calloc(strlen(guess) + 1, sizeof(char));
    strncpy(temp1, guess, strlen(guess) + 1);
    char* temp2 = calloc(strlen(target) + 1, sizeof(char));
    strncpy(temp2, target, strlen(target) + 1);

    int ltrs = 0;
    int plcmt = 0;
    capitalize(temp1);
    capitalize(temp2);

    for (int i = 0; i < strlen(temp1); i++) {
        if (temp1[i] == temp2[i]) {
            ltrs++;
            plcmt++;
            temp2[i] = tolower(temp2[i]); // lowercase letters indicate visited status
        }
    }

    if (ltrs != strlen(temp2)) {
        for (int i = 0; i < strlen(temp1); i++) {
            for (int j = 0; j < strlen(temp2); j++) {
                if (temp1[i] == temp2[j]) {
                    ltrs++;
                    temp2[j] = tolower(temp2[j]); // lowercase letters indicate visited status
                }
            }
        }
    }

    free(temp1);
    free(temp2);
    *correct_letters = ltrs;
    *correct_placement = plcmt;
}


int main(int argc, char** argv)
{
    setvbuf( stdout, NULL, _IONBF, 0 );

    // Argument checking
    if (argc != 5) {
        fprintf(stderr, "ERROR: Incorrect arguments!\n");
        fprintf(stderr, "USAGE: <seed> <port> <dictionary_file> <longest_word_length>\n");
        return EXIT_FAILURE;
    }

    int i = 0;
    while (argv[1][i]) {
        if (!isdigit(argv[1][i])) {
            fprintf(stderr, "ERROR: Argument 1 is not an unsigned integer!\n");
            return EXIT_FAILURE;
        }
        i++;
    }
    unsigned int seed = atoi(argv[1]);

    i = 0;
    while (argv[2][i]) {
        if (!isdigit(argv[2][i])) {
            fprintf(stderr, "ERROR: Argument 2 is not an unsigned short!\n");
            return EXIT_FAILURE;
        }
        i++;
    }

    if (atoi(argv[2]) < 0 || atoi(argv[2]) > 65535) {
        fprintf(stderr, "ERROR: Argument 2 is an invalid port number!\n");
        return EXIT_FAILURE;
    } 
    unsigned short port = atoi(argv[2]);
    
    int dictfd = open(argv[3], O_RDONLY);
    if (dictfd == -1) {
        fprintf(stderr, "ERROR: Argument 3 is an invalid file!\n");
        return EXIT_FAILURE;
    }

    i = 0;
    while (argv[4][i]) {
        if (!isdigit(argv[4][i])) {
            fprintf(stderr, "ERROR: Argument 4 is not an unsigned integer!\n");
            return EXIT_FAILURE;
        }
        i++;
    }

    if (atoi(argv[4]) <= 0 || atoi(argv[4]) > 1024) {
        fprintf(stderr, "ERROR: Argument 4 is an invalid word length!\n");
        return EXIT_FAILURE;
    }
    int longest_word_length = atoi(argv[4]);

    #ifdef DEBUG_MODE
    printf("Parsed arguments:\n");
    printf("\tseed: %d\n", seed);
    printf("\tport: %d\n", port);
    printf("\tfile: \'%s\' on descriptor %d\n", argv[3], dictfd);
    printf("\tlongest word length: %d\n", longest_word_length);
    #endif

    int dictionary_size = 0;
    char** dictionary = parseDict(dictfd, longest_word_length, &dictionary_size);
    srand(seed);
    char* secret_word = calloc(longest_word_length, sizeof(char));
    memccpy(secret_word, dictionary[rand() % dictionary_size], '\0', longest_word_length);
    
    
    fd_set readfds;
    int client_sockets[MAX_CLIENTS]; /* client socket fd list */
    int client_socket_index = 0;     /* next free spot */
    char** usernames = calloc(MAX_CLIENTS, sizeof(char*));
    int client_username_index = 0;

    /* Create the listener socket as TCP socket (SOCK_STREAM) */
    int listener = socket(PF_INET, SOCK_STREAM, 0);
    /* here, the listener is a socket descriptor (part of the fd table) */

    if (listener == -1)
    {
        fprintf(stderr, "ERROR: socket() failed!\n");
        return EXIT_FAILURE;
    }

    /* populate the socket structure for bind() */
    struct sockaddr_in server;
    server.sin_family = PF_INET; /* AF_INET */

    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* allow any IP address to connect */


    /* htons() is host-to-network short for data marshalling */
    /* Internet is big endian; Intel is little endian; etc.  */
    server.sin_port = htons(port);
    int len = sizeof(server);

    /* attempt to bind (or associate) port with the socket */
    if (bind(listener, (struct sockaddr *)&server, len) == -1)
    {
        server.sin_port = htons(0);
        if (bind(listener, (struct sockaddr *)&server, len) == -1) {
            fprintf(stderr, "ERROR: Unable to bind listener socket to a port!\n");
            return EXIT_FAILURE;
        }
    }

    /* identify this port as a TCP listener port */
    if (listen(listener, MAX_CLIENTS) == -1)
    {
        fprintf(stderr, "ERROR: Unable to mark socket as a listener!\n");
        return EXIT_FAILURE;
    }

    printf("SERVER: TCP listener socket (fd %d) bound to port %d\n", listener, port);

    int n, bytes_written = 0;
    char buffer[MAXBUFFER + 1];
    char* temp = calloc(5, sizeof(char)); // used to send integers as strings to clients
    

    while (1)
    {
        printf("Secret word is: %s\n", secret_word);
        FD_ZERO(&readfds);
        if (client_socket_index > MAX_CLIENTS) {
            fprintf(stderr, "ERROR: Too many clients connected!\n");
            return EXIT_FAILURE;
        } else if (client_socket_index == MAX_CLIENTS) {
            printf("SERVER: Max number of clients reached (%d clients) - ", client_socket_index);
            printf("will not be listening for additional connection requests\n");
        } else {
            FD_SET(listener, &readfds); /* listener socket, fd 3 */
            printf("SERVER: Set FD_SET to include listener fd %d\n", listener);
        }

        /* initially, this for loop does nothing; but once we have some */
        /*  client connections, we will add each client connection's fd */
        /*   to the readfds (the FD set) */
        for (int i = 0; i < client_socket_index; i++)
        {
            FD_SET(client_sockets[i], &readfds);
            printf("SERVER: Set FD_SET to include client socket fd %d\n", client_sockets[i]);
        }

        printf("SERVER: Blocked on select()...\n");

        /* This is a BLOCKING call, but will block on all readfds */
        int ready = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);

        /* ready is the number of ready file descriptors */
        printf("SERVER: select() identified %d descriptor(s) with activity\n", ready);

        /* is there activity on the listener descriptor? */
        if (FD_ISSET(listener, &readfds))
        {
            struct sockaddr_in client;
            int fromlen = sizeof(client);

            /* we know this accept() call will not block! */
            int newsd = accept(listener, (struct sockaddr *)&client, (socklen_t *)&fromlen);
            if (newsd == -1)
            {
                perror("accept() failed");
                continue;
            }
            n = send(newsd, "Welcome to Guess the Word, please enter your username.\n", 55, 0);
            if (n == -1) {
                fprintf(stderr, "ERROR: send() failed\n");
                return EXIT_FAILURE;
            }

            printf("SERVER: Accepted client connection from %s\n", inet_ntoa((struct in_addr)client.sin_addr));
            client_sockets[client_socket_index++] = newsd;
        }

        /* is there activity on any of the established connections? */
        for (int i = 0; i < client_socket_index; i++)
        {
            int fd = client_sockets[i];

            if (FD_ISSET(fd, &readfds))
            {
                /* we know this recv() call will not block! */
                n = recv(fd, buffer, MAXBUFFER - 1, 0); /* or read() */

                if (n == -1)
                {
                    perror("recv() failed");
                    return EXIT_FAILURE;
                }
                else if (n == 0)
                {
                    printf("SERVER: Rcvd 0 from recv(); closing client socket...\n");
                    close(fd);

                    /* remove fd from client_sockets[] array: */
                    for (int k = 0; k < client_socket_index; k++)
                    {
                        if (fd == client_sockets[k])
                        {
                            free(usernames[k]);
                            /* found it -- copy remaining elements over fd */
                            for (int m = k; m < client_socket_index - 1; m++)
                            {
                                client_sockets[m] = client_sockets[m + 1];
                                usernames[m] = usernames[m + 1];
                            }
                            client_username_index--;
                            client_socket_index--;
                            usernames[client_username_index] = NULL;
                            break; /* all done */
                        }
                    }
                }
                else /* n > 0 */
                {
                    if (n > longest_word_length + 1) { // +1 accounts for newline byte ('\n')
                        fprintf(stderr, "ERROR: Too many bytes sent!\n");
                        return EXIT_FAILURE;
                    }
                    buffer[n - 1] = '\0'; /* assume this is text data, overrites newline with null byte */
                    printf("SERVER: Rcvd message from fd %d: [%s]\n", fd, buffer);
                    printf("SERVER: Sending acknowledgement to client\n");
                    if (usernames[i] == NULL) { // Need to get username from client
                        if (checkUsernames(buffer, usernames)) {
                            send(fd, "Let's start playing, ", 21, 0);
                            send(fd, buffer, n - 1, 0);
                            send(fd, "\nThere are ", 11, 0);
                            if ((bytes_written = snprintf(temp, 5, "%d", client_username_index + 1)) < 0) {
                                fprintf(stderr, "ERROR: snprintf() failed\n");
                                return EXIT_FAILURE;
                            }
                            send(fd, temp, bytes_written, 0);
                            send(fd, " player(s) playing. The secret word is ", 39, 0);
                            if ((bytes_written = snprintf(temp, 5, "%ld", strlen(secret_word))) < 0) {
                                fprintf(stderr, "ERROR: snprintf() failed\n");
                                return EXIT_FAILURE;
                            }
                            send(fd, temp, bytes_written, 0);
                            send(fd, " letter(s).\n", 12, 0);

                            usernames[i] = calloc(n - 1, sizeof(char));
                            memcpy(usernames[i], buffer, n - 1);
                            client_username_index++;
                        } else {
                            send(fd, "Username ", 9, 0);
                            send(fd, buffer, n - 1, 0);
                            send(fd, " is already taken, please enter a different username\n", 53, 0);
                        }
                    } else { // client is providing a guess
                        if (strlen(secret_word) == strlen(buffer)) {
                            if (isSame(buffer, secret_word)) { // correct guess was made
                                for (int a = 0; a < client_socket_index; a++) {
                                    int sck = client_sockets[a];
                                    send(sck, usernames[i], strlen(usernames[i]), 0);
                                    send(sck, " has correctly guessed the word ", 32, 0);
                                    send(sck, secret_word, strlen(secret_word), 0);
                                    send(sck, "\n", 1, 0);
                                    close(sck);
                                }
                                client_socket_index = 0;
                                for (int a = 0; a < client_username_index; a++) {
                                    free(usernames[a]);
                                    usernames[a] = NULL;
                                }
                                client_username_index = 0;
                                memccpy(secret_word, dictionary[rand() % dictionary_size], '\0', longest_word_length);
                            } else {
                                int correct_letters = 0;
                                int correct_placed = 0;
                                compareWords(buffer, secret_word, &correct_letters, &correct_placed);
                                for (int a = 0; a < client_socket_index; a++) {
                                    int sck = client_sockets[a];
                                    send(sck, usernames[i], strlen(usernames[i]), 0);
                                    send(sck, " guessed ", 9, 0);
                                    send(sck, buffer, strlen(buffer), 0);
                                    send(sck, ": ", 2, 0);
                                    if ((bytes_written = snprintf(temp, 5, "%d", correct_letters)) < 0) {
                                        fprintf(stderr, "ERROR: snprintf() failed\n");
                                        return EXIT_FAILURE;
                                    }
                                    send(sck, temp, bytes_written, 0);
                                    send(sck, " letter(s) were correct and ", 28, 0);
                                    if ((bytes_written = snprintf(temp, 5, "%d", correct_placed)) < 0) {
                                        fprintf(stderr, "ERROR: snprintf() failed\n");
                                        return EXIT_FAILURE;
                                    }
                                    send(sck, temp, bytes_written, 0);
                                    send(sck, " letter(s) were correctly placed.\n", 34, 0);
                                }
                            }
                        } else {
                            send(fd, "Invalid guess length. The secret word is ", 41, 0);
                            if ((bytes_written = snprintf(temp, 5, "%ld", strlen(secret_word))) < 0) {
                                fprintf(stderr, "ERROR: snprintf() failed\n");
                                return EXIT_FAILURE;
                            }
                            send(fd, temp, bytes_written, 0);
                            send(fd, " letter(s).\n", 12, 0);
                        }
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}