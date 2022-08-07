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

#define MAXBUFFER 8192
#define MAX_CLIENTS 5


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

    #ifdef DEBUG_MODE
    printf("Printing contents of dictionary:\n");
    for (int i = 0; i < num_words; i++) {
        printf("%s\n", dict[i]);
    }
    #endif
    
    return dict;
}


int main(int argc, char** argv)
{
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
    
    fd_set readfds;
    int client_sockets[MAX_CLIENTS]; /* client socket fd list */
    int client_socket_index = 0;     /* next free spot */

    /* Create the listener socket as TCP socket (SOCK_STREAM) */
    int listener = socket(PF_INET, SOCK_STREAM, 0);
    /* here, the listener is a socket descriptor (part of the fd table) */

    if (listener == -1)
    {
        perror("socket() failed");
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
        perror("bind() failed");
        return EXIT_FAILURE;
    }

    /* identify this port as a TCP listener port */
    if (listen(listener, 5) == -1)
    {
        perror("listen() failed");
        return EXIT_FAILURE;
    }

    printf("SERVER: TCP listener socket (fd %d) bound to port %d\n", listener, port);

    int n;
    char buffer[MAXBUFFER];

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds); /* listener socket, fd 3 */
        printf("SERVER: Set FD_SET to include listener fd %d\n", listener);

        /* initially, this for loop does nothing; but once we have some */
        /*  client connections, we will add each client connection's fd */
        /*   to the readfds (the FD set) */
        for (int i = 0; i < client_socket_index; i++)
        {
            FD_SET(client_sockets[i], &readfds);
            printf("SERVER: Set FD_SET to include client socket fd %d\n", client_sockets[i]);
        }

        printf("SERVER: Blocked on select()...\n");
#if 1
        /* This is a BLOCKING call, but will block on all readfds */
        int ready = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
#endif


        /* ready is the number of ready file descriptors */
        printf("SERVER: select() identified %d descriptor(s) with activity\n", ready);

        /* is there activity on the listener descriptor? */
        if (FD_ISSET(listener, &readfds))
        {
            struct sockaddr_in client;
            int fromlen = sizeof(client);

            /* we know this accept() call will not block! */
            /* printf( "SERVER: Blocked on accept()\n" ); */
            int newsd = accept(listener, (struct sockaddr *)&client, (socklen_t *)&fromlen);

            if (newsd == -1)
            {
                perror("accept() failed");
                continue;
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
                /* printf( "SERVER: Blocked on recv()\n" ); */
                /* recv() call will block until we receive data (n > 0)
                    or an error occurs (n == -1)
                     or the client closed its socket (n == 0) */
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
                            /* found it -- copy remaining elements over fd */
                            for (int m = k; m < client_socket_index - 1; m++)
                            {
                                client_sockets[m] = client_sockets[m + 1];
                            }
                            client_socket_index--;
                            break; /* all done */
                        }
                    }
                }
                else /* n > 0 */
                {
                    buffer[n] = '\0'; /* assume this is text data */
                    printf("SERVER: Rcvd message from: [%s]\n", buffer);

                    printf("SERVER: Sending acknowledgement to client\n");
                    /* send OK message back to client */
                    n = send(fd, "OK\n", 3, 0);

                    if (n == -1)
                    {
                        perror("send() failed");
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}