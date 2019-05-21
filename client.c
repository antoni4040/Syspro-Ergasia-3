// Created by Antonis Karvelas.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char** argv)
{
    char* dirname;
    int port, workerThreads, bufferSize, serverPort;

    for(int i = 0; i < argc; i++)
    {
        // Get directory name:
        if(strcmp(argv[i], "-d") == 0)
        {
            i++;
            dirname = malloc(strlen(argv[i]) * sizeof(char));
            strcpy(dirname, argv[i]);
        }
        // Get port number:
        if(strcmp(argv[i], "-p") == 0)
        {
            i++;
            port = atoi(argv[i]);
        }
        // Get worker threads:
        if(strcmp(argv[i], "-w") == 0)
        {
            i++;
            workerThreads = atoi(argv[i]);
        }
        // Get buffer size:
        if(strcmp(argv[i], "-b") == 0)
        {
            i++;
            bufferSize = atoi(argv[i]);
        }
        // Get server port number:
        if(strcmp(argv[i], "-sp") == 0)
        {
            i++;
            serverPort = atoi(argv[i]);
        }
    }

    int clientSocket;
    struct sockaddr_in  server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *rem = gethostbyname("localhost");

    // Create socket:
    if((clientSocket = socket(AF_INET , SOCK_STREAM , 0)) < 0)
    {
        perror("Error creating socket.");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, rem->h_addr_list[0], rem->h_length);

    // Initialize connection:
    if(connect(clientSocket, serverptr ,sizeof(server)) < 0)
    {
        perror("Error connecting.");
    }

    return 0;
}