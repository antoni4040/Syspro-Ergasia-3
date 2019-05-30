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

#include "linked_list.h"
#include "requests.h"

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

    // int clientSocket;
    // struct sockaddr_in  client;
    // struct sockaddr *clientptr = (struct sockaddr*)&client;
    // struct hostent *rem = gethostbyname("localhost");

    // client.sin_family = AF_INET;
    // memcpy(&client.sin_addr, rem->h_addr_list[0], rem->h_length);
    // client.sin_port = htons(serverPort);

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

    Socket* clientSocket = initializeSocket(serverPort, address);

    // // Create socket:
    // if((clientSocket = socket(AF_INET , SOCK_STREAM , 0)) < 0)
    // {
    //     perror("Error creating socket.");
    //     exit(EXIT_FAILURE);
    // }


    // Initialize connection:
    if(connect(clientSocket->socket, (struct sockaddr*)&clientSocket->socketAddress ,sizeof(clientSocket->socketAddress)) < 0)
    {
        perror("Error connecting client to server.");
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
    strcat(logOnRequest, "GET_CLIENTS ");
    strcat(logOnRequest, addressString);
    strcat(logOnRequest, " ");
    strcat(logOnRequest, portString);
    strcat(logOnRequest, " ");

    // Send initial request for login and get clients:
    send(clientSocket->socket, logOnRequest, 1024, 0);
    close(clientSocket->socket);

    // Setup server socket:
    Socket* serverSocket = initializeSocket(port, INADDR_ANY);
    printf("client port aa: %d\n", serverSocket->socketAddress.sin_port);
    
    char* buffer = malloc((serverSocket->socketSize+1) * sizeof(char));

    if(bind(serverSocket->socket, (struct sockaddr*)&serverSocket->socketAddress, sizeof(serverSocket->socketAddress)) < 0)
    {
        perror("Error binding client socket.");
        exit(EXIT_FAILURE);
    }

    // Shut-up and LISTEN:
    if(listen(serverSocket->socket, 5) < 0)
    {
        perror("Error listening to port.");
        exit(EXIT_FAILURE);
    }

    // Setup new socket for connection:
    Socket* newSocket = malloc(sizeof(Socket));
    socklen_t socklen;

    // Listen for client requests:
    while(1)
    {
        // Accept client connection:
        if((newSocket->socket = accept(serverSocket->socket, (struct sockaddr*)&newSocket->socketAddress, &socklen)) < 0) 
        {
            perror("Error accepting connection.");
            exit(EXIT_FAILURE);
        }
        // Receive request:
        if((recv(newSocket->socket, buffer, serverSocket->socketSize, 0)) < 0)
        {
            perror("Error in recvfrom.");
            exit(EXIT_FAILURE);
        }
        printf("Client buffer: %s\n", buffer);
        close(newSocket->socket);
    }

    return 0;
}