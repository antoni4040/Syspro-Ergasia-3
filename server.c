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

    // Create clients linked list:
    LinkedList* clientList = initializeLinkedList();

    // Setup socket:
    int serverSocket;
    struct sockaddr_in server;
    struct sockaddr *serverptr =(struct sockaddr*)&server;

    if((serverSocket = socket(AF_INET , SOCK_STREAM , 0)) < 0)
    {
        perror("Error creating socket.");
        exit(EXIT_FAILURE);
    }

    // Set socket options:
    int size;
    unsigned int socklen = sizeof(size);
    getsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, &size, &socklen);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    printf("buffer size %d\n", size);
    char* buffer = malloc((size+1) * sizeof(char));

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

    // Setup new socket for connection:
    int newSocket;
    struct sockaddr_in client;
    socklen_t  clientlen;
    struct sockaddr *clientptr =(struct sockaddr*)&client;
    char* requestString;

    // Listen for client requests:
    while(1)
    {
        // Accept client connection:
        if((newSocket = accept(serverSocket, clientptr, &clientlen)) < 0) 
        {
            perror("Error accepting connection.");
            exit(EXIT_FAILURE);
        }
        printf("Accepted connection.\n");

        // Receive request:
        if((recv(newSocket, buffer, size, 0)) < 0)
        {
            perror("Error in recvfrom.");
            exit(EXIT_FAILURE);
        }
        printf("buffer: %s\n", buffer);

        char* bufferCopy = buffer;
        while((requestString = strtok(bufferCopy, " ")) != NULL)
        {
            // LOG_ON request:
            if(strcmp(requestString, "LOG_ON") == 0)
            {
                printf("LOG_ON request received.\n");
                // Get strings of binary represantation:
                char* IPString = strtok(NULL, " ");
                char* PortString = strtok(NULL, " ");

                // Convert them to 32bit and 16bit values:
                uint32_t ip = strtol(IPString, NULL, 16);
                ip = ntohl(ip);
                uint16_t port = strtol(PortString, NULL, 16);
                port = ntohs(port);
                // Search for client in clients list:
                int found = checkClientInLinkedList(clientList, port, ip);

                // If it doesn't exist, add client to list:
                if(found == 1)
                {
                    Client* newClient = initializeClient(port, ip);
                    Node* newNode = initializeNode(newClient);
                    appendToLinkedList(clientList, newNode);
                    printf("Added new client to list.\n");
                }

                // char* ipPrintable;
                // inet_ntop(AF_INET, &ip, ipPrintable, INET_ADDRSTRLEN);
            }
            // GET_CLIENTS request:
            else if(strcmp(requestString, "GET_CLIENTS") == 0)
            {
                printf("GET_CLIENTS request received.\n");
                char clientIP[INET_ADDRSTRLEN]; 
                inet_ntop(AF_INET, &client.sin_addr, clientIP, INET_ADDRSTRLEN);
                int clientPort = ntohs(client.sin_port);
                printf("Client ip: %s Client port: %d\n", clientIP, clientPort);
                sendClientList(1, clientList, size);
            }
            // LOG_OFF request:
            else if(strcmp(requestString, "LOG_OFF") == 0)
            {
                printf("LOG_OFF request received.\n");
            }
            bufferCopy = NULL;
        }
        close(newSocket);
        printf("Closed connection.\n");            
    }

    free(buffer);
    return 0;
}