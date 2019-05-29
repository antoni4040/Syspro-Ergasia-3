// Created by Antonis Karvelas.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>

int main(int argc, char** argv)
{
    char *dirname, *serverIPString;
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
        // Get server IP:
        if(strcmp(argv[i], "-sip") == 0)
        {
            i++;
            serverIPString = malloc(strlen(argv[i]) * sizeof(char));
            strcpy(serverIPString, argv[i]);
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
    server.sin_port = htons(serverPort);

    // Convert ip string to binary:
    uint32_t address;
    inet_pton(AF_INET, serverIPString, &address);
    // Convert ip to network order:
    address = htonl(address);
    // Make it a string:
    char* addressString = malloc(8 * sizeof(char));
    sprintf(addressString, "%x", address);

    // Convert ip to network order:
    uint16_t portBinary = htons(port);
    // Make it a string:
    char* portString = malloc(4 * sizeof(char));
    sprintf(portString, "%x", portBinary);

    // Initialize connection:
    if(connect(clientSocket, serverptr ,sizeof(server)) < 0)
    {
        perror("Error connecting.");
        exit(EXIT_FAILURE);
    }

    // Inform server about our existence:
    char logOnRequest[1024];
    strcpy(logOnRequest, "LOG_ON ");
    strcat(logOnRequest, addressString);
    strcat(logOnRequest, " ");
    strcat(logOnRequest, portString);
    strcat(logOnRequest, " ");

    // Ask for connected clients:
    strcat(logOnRequest, "GET_CLIENTS");

    // printf("%s\n", logOnRequest);
    send(clientSocket, logOnRequest, 1024, 0);

    return 0;
}