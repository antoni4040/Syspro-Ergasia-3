// Created by Antonis Karvelas.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char* argv[])
{
    int port;
    if(argc != 3)
    {
        fprintf(stderr, "Wrong command line parameters number. Well done.\n");
        return 0;
    }
    if(strcmp(argv[1], "-p") == 0)
    {
        port = atoi(argv[2]);
    }
    else
    {
        fprintf(stderr, "Wrong command line parameters. You're not very clever, are you?\n");
        return 0;
    }
    printf("Starting server at port %d.\n", port);

    // Setup socket:
    int serverSocket;
    struct sockaddr_in server;
    struct sockaddr *serverptr =(struct sockaddr  *)&server;

    if((serverSocket = socket(AF_INET , SOCK_STREAM , 0)) < 0)
    {
        perror("Error creating socket.");
        exit(EXIT_FAILURE);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if(bind(serverSocket , serverptr, sizeof(server)) < 0)
    {
        perror("Error binding socket.");
        exit(EXIT_FAILURE);
    }

    // Shut-up and LISTEN:
    if(listen(serverSocket , 5) < 0)
    {
        perror("Error listening to port.");
        exit(EXIT_FAILURE);
    }
    printf("Listening  for  connections  to port %d\n", port);

    // Listen for client requests:
    while(1)
    {

    }

    return 0;
}