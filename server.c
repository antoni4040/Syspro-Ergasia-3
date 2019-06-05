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
        port = ntohs(atoi(argv[2]));
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
    Socket* serverSocket = initializeSocket(port, INADDR_ANY);
    
    char* buffer = malloc((serverSocket->socketSize+1) * sizeof(char));


    if(bind(serverSocket->socket, (struct sockaddr *) &serverSocket->socketAddress, sizeof(serverSocket->socketAddress)) < 0)
    {
        perror("Error binding server socket.");
        exit(EXIT_FAILURE);
    }

    // Shut-up and LISTEN:
    if(listen(serverSocket->socket, 5) < 0)
    {
        perror("Error listening to port.");
        exit(EXIT_FAILURE);
    }
    printf("Listening  for  connections  to port %d\n", port);

    // Setup new socket for connection:
    Socket* newSocket = malloc(sizeof(newSocket));
    socklen_t socklen;
    char* requestString;

    // Listen for client requests:
    while(1)
    {
        // Accept client connection:
        if((newSocket->socket = accept(serverSocket->socket, (struct sockaddr*)&newSocket->socketAddress, &socklen)) < 0) 
        {
            perror("Error accepting server connection.");
            exit(EXIT_FAILURE);
        }
        printf("Accepted connection.\n");

        // Receive request:
        if((recv(newSocket->socket, buffer, serverSocket->socketSize, 0)) < 0)
        {
            perror("Error in recvfrom.");
            exit(EXIT_FAILURE);
        }
        close(newSocket->socket);
        // printf("buffer: %s\n", buffer);

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
                // ip = ntohl(ip);
                uint16_t port = strtol(PortString, NULL, 16);
                // port = ntohs(port);
                // Search for client in clients list:
                int found = checkClientInLinkedList(clientList, port, ip);
                

                // If it doesn't exist, send USER_ON signal to all clients and add client to list:
                if(found == 1)
                {
                    sendUserOn(clientList, IPString, PortString);
                    Client* newClient = initializeClient(port, ip);
                    Node* newNode = initializeNode(newClient);
                    appendToLinkedList(clientList, newNode);
                    printf("Added new client to list.\n");
                }
            }
            // GET_CLIENTS request:
            else if(strcmp(requestString, "GET_CLIENTS") == 0)
            {
                printf("GET_CLIENTS request received.\n");
                // Get partner ip and listening port:
                char* IPString = strtok(NULL, " ");
                char* PortString = strtok(NULL, " ");
                uint32_t partnerIP = strtol(IPString, NULL, 16);
                // partnerIP = ntohl(partnerIP);
                uint16_t partnerPort = strtol(PortString, NULL, 16);
                // partnerPort = ntohs(partnerPort);
                // printf("partner port: %d\n", htons(partnerPort));
                Socket* sendListSocket = initializeSocket(partnerPort, partnerIP);
                sendClientList(sendListSocket, clientList, sendListSocket->socketSize);
                close(sendListSocket->socket);
            }
            // LOG_OFF request:
            else if(strcmp(requestString, "LOG_OFF") == 0)
            {
                printf("LOG_OFF request received.\n");
            }
            bufferCopy = NULL;
        }
        printf("Closed connection.\n");    
    }
    free(buffer);
    return 0;
}