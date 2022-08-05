/* tcp-server-iterative.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXBUFFER 512

int main(int argc, char **argv)
{
    setvbuf( stdout, NULL, _IONBF, 0 );
    /* Create the listener socket as TCP socket (SOCK_STREAM) */
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    /* here, the listener is a socket descriptor (part of the fd table) */

    if (listener == -1)
    {
        perror("socket() failed");
        return EXIT_FAILURE;
    }

    /* populate the socket structure for bind() */
    struct sockaddr_in tcp_server;
    tcp_server.sin_family = AF_INET; /* IPv4 */

    tcp_server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* allow any IP address to connect */

    int port;
    if (argc == 2)
    {
        if (atoi(argv[1]) >= 0 && atoi(argv[1]) <= 65535)
        {
            port = atoi(argv[1]);
        }
        else
        {
            port = 0;
        }
    }
    else
    {
        port = 0;
    }

    /* htons() is host-to-network short for data marshalling */
    /* Internet is big endian; Intel is little endian; etc.  */
    tcp_server.sin_port = htons(port);

    int length = sizeof(tcp_server);

    /* attempt to bind (or associate) port with the socket */
    if (bind(listener, (struct sockaddr *)&tcp_server, length) == -1)
    {
        tcp_server.sin_port = htons(0);
        bind(listener, (struct sockaddr *)&tcp_server, length);
    }

    /* identify this port as a TCP listener port */
    if (listen(listener, 5) == -1)
    {
        perror("listen() failed");
        return EXIT_FAILURE;
    }

    printf("SERVER: TCP listener socket (fd %d) bound to port %d\n", listener, port);

    while (1)
    {
        struct sockaddr_in remote_client;
        int addrlen = sizeof(remote_client);

        printf("SERVER: Blocked on accept()\n");
        int newsd = accept(listener, (struct sockaddr *)&remote_client, (socklen_t *)&addrlen);
        if (newsd == -1)
        {
            perror("accept() failed");
            continue;
        }

        /* the listener variable above is the listener socket descriptor that
            we use to accept new incoming client connections on port 8123 */

        /* the newsd variable is the means of communicating via recv()/send()
            or read()/write() calls with the connected remote client */

        printf("SERVER: Accepted new client connection on newsd %d\n", newsd);

        /* we have successfully established a TCP connection between server
            and a remote client; below implements the application protocol */

        int n, count = 0;

        do
        {
            char buffer[MAXBUFFER + 1];

            printf("SERVER: Blocked on recv()\n");

            /* recv() call will block until we receive data (n > 0)
                or an error occurs (n == -1)
                 or the client closed its socket (n == 0) */
            n = recv(newsd, buffer, MAXBUFFER, 0); /* or read() */

            if (n == -1)
            {
                perror("recv() failed");
                return EXIT_FAILURE;
            }
            else if (n == 0)
            {
                printf("SERVER: Rcvd 0 from recv(); closing descriptor %d...\n", newsd);
            }
            else /* n > 0 */
            {
                buffer[n] = '\0'; /* assume this is text data */
                printf("SERVER: Rcvd message (%d bytes) from %s: [%s]\n", n,
                       inet_ntoa((struct in_addr)remote_client.sin_addr),
                       buffer);

                for (int i = 0; i < n; i++) {
                    if (buffer[i] == 'G') {
                        count++;
                    }
                }
                
                
            }
        } while (n > 0);

        /* at this point, the remote client side has closed its socket */
        printf("SERVER: Sending acknowledgement to client\n");
        /* sends number of G's counted message back to client */
        uint32_t relay = htonl(count);
        n = send(newsd, &relay, 4, 0);
        if (n == -1)
        {
            perror("send() failed");
            return EXIT_FAILURE;
        }

        close(newsd);
    }

    close(listener);

    return EXIT_SUCCESS;
}
