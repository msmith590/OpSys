/* udp-server.c */

/* UDP SERVER example
   -- socket()             create a socket (endpoint) for communication
   -- bind()               bind to (assign) a specific port number
                           (or let the OS assign us a port number)
   -- getsockname()        get socket "name" -- IP address, port number, etc.
   -- recvfrom()/sendto()  receive/send datagrams
*/

/* To test this server, you can use the following
   command-line netcat tool:


   bash$ netcat -u linux02.cs.rpi.edu 41234
                   ^^^^^^
                   replace with your hostname

   Note that netcat will act as a client to this UDP server...

   The hostname (e.g., linux02.cs.rpi.edu) can also be
   localhost (127.0.0.1); and the port number must match what
   the server reports.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXBUFFER 512

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    int sd; /* socket descriptor -- this is actually in the fd table! */

    /* create the socket (endpoint) on the server side */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == -1)
    {
        perror("socket() failed");
        return EXIT_FAILURE;
    }

    printf("Server-side UDP socket created on descriptor %d\n", sd);

    struct sockaddr_in udp_server;
    int length = sizeof(udp_server);

    udp_server.sin_family = AF_INET; /* IPv4 */

    udp_server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* any remote IP can send us a datagram */

    int port;
    if (argc == 2)
    {
        if (atoi(argv[1]) >= 0 && atoi(argv[1]) <= 65535) {
            port = atoi(argv[1]);
        } else {
            port = 0;
        }
    }
    else
    {
        port = 0;
    }

    /* specify the port number for the server */
    udp_server.sin_port = htons(port); /* a 0 here means let the kernel assign
                                        us a port number to listen on */

    /* TO DO: change the 0 above to an actual port number (e.g., 8123) */

    /* bind to a specific (OS-assigned) port number */
    if (bind(sd, (struct sockaddr *)&udp_server, length) == -1)
    {
        udp_server.sin_port = htons(0);
        bind(sd, (struct sockaddr *)&udp_server, length);
    }

    /* call getsockname() to obtain the port number that was just assigned */
    if (getsockname(sd, (struct sockaddr *)&udp_server, (socklen_t *)&length) == -1)
    {
        perror("getsockname() failed");
        return EXIT_FAILURE;
    }

    printf("UDP server bound to port number %d\n", ntohs(udp_server.sin_port));
    /* the code below implements the application protocol */
    while (1)
    {
        int count = 0;
        char buffer[MAXBUFFER + 1];
        struct sockaddr_in remote_client;
        int addrlen = sizeof(remote_client);
        /* read a datagram from the remote client side (BLOCKING) */
        int n = recvfrom(sd, buffer, MAXBUFFER, 0,
                         (struct sockaddr *)&remote_client, (socklen_t *)&addrlen);

        if (n == -1)
        {
            perror("recvfrom() failed");
            continue;
        }

        printf("Rcvd datagram from %s port %d\n",
               inet_ntoa(remote_client.sin_addr), ntohs(remote_client.sin_port));

        printf("RCVD %d bytes\n", n);
        buffer[n] = '\0'; /* assume that its printable char[] data */
        printf("RCVD: [%s]\n", buffer);

        for (int i = 0; i < n; i++) {
            if (buffer[i] == 'G') {
                count++;
            }
        }
        /* TO DO: check the return value from sendto() */
        
        uint32_t relay = htonl(count);
        sendto(sd, &relay, 4, 0, (struct sockaddr *)&remote_client, addrlen);
        
    }
    
    close(sd);
    return EXIT_SUCCESS;
}