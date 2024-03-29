// Created by Antonis Karvelas.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>

#include "linked_list.h"
#include "requests.h"
#include "threads.h"

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
            port = ntohs(atoi(argv[i]));
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
            serverPort = ntohs(atoi(argv[i]));
        }
        // Get server IP:
        if(strcmp(argv[i], "-sip") == 0)
        {
            i++;
            serverIPString = malloc(strlen(argv[i]) * sizeof(char));
            strcpy(serverIPString, argv[i]);
        }
    }

    // Create clients linked list:
    LinkedList* clientList = initializeLinkedList();

    // Convert ip string to binary:
    uint32_t address;
    inet_pton(AF_INET, serverIPString, &address);
    // Convert ip to network order:
    address = ntohl(address);
    // Make it a string:
    char* addressString = malloc(8 * sizeof(char));
    sprintf(addressString, "%x", address);

    // Convert ip to network order:
    // uint16_t portBinary = htons(port);
    // Make it a string:
    char* portString = malloc(4 * sizeof(char));
    sprintf(portString, "%x", port);

    // Add this client to the list:
    Client* newClient = initializeClient(port, address);
    Node* newNode = initializeNode(newClient);
    appendToLinkedList(clientList, newNode);

    Socket* clientSocket = initializeSocket(serverPort, address);

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
    Socket* serverSocket = initializeSocket(port, address);
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
    char* requestString;
    fd_set socketSet;

    // Initialize round buffer:
    roundBuffer* threadRoundBuffer = initializeRoundBuffer(bufferSize);

    // Initialize threadpool:
    Threadpool* threads = initializeThreadpool(workerThreads, threadRoundBuffer, address, port, clientList);

    // Start accepting requests:
    while(1)
    {
        FD_ZERO(&socketSet);
        FD_SET(serverSocket->socket, &socketSet);
        select(sizeof(socketSet)*8, &socketSet, NULL, NULL, NULL);
        // bzero(buffer, serverSocket->socketSize);
        memset(buffer, 0, sizeof(char)*serverSocket->socketSize);
        // Accept client connection:
        if((newSocket->socket = accept(serverSocket->socket, (struct sockaddr*)&newSocket->socketAddress, &socklen)) < 0) 
        {
            perror("Error accepting client connection.");
            exit(EXIT_FAILURE);
        }
        // Receive request:
        if((recv(newSocket->socket, buffer, serverSocket->socketSize, 0)) < 0)
        {
            perror("Error in recvfrom.");
            exit(EXIT_FAILURE);
        }
        printf("Client buffer: Dir:%s\t\t\t%s\n", dirname, buffer);

        // Get clients list from server:
        char* bufferCopy = buffer;
        while((requestString = strtok(bufferCopy, " ")) != NULL)
        {
            // Client list information:
            if(strcmp(requestString, "CLIENT_LIST") == 0)
            {
                printf("CLIENT_LIST response received.\n");
                char* numString = strtok(NULL, " ");
                int numOfClients = atoi(numString);
                for(int i = 0; i < numOfClients; i++)
                {
                    // Get strings of binary represantation:
                    char* IPString = strtok(NULL, " ");
                    char* PortString = strtok(NULL, " ");

                    // Convert them to 32bit and 16bit values:
                    uint32_t clientIP = strtol(IPString, NULL, 16);
                    // clientIP = ntohl(clientIP);
                    uint16_t clientPort = strtol(PortString, NULL, 16);
                    // clientPort = ntohs(clientPort);

                    // Search for client in clients list:
                    int found = checkClientInLinkedList(clientList, clientPort, clientIP);

                    // If it doesn't exist, add client to list:
                    if(found == 1)
                    {
                        printf("------Adding to rb: %d %d\n", htons(clientPort), htons(port));
                        Client* newClient = initializeClient(clientPort, clientIP);
                        Node* newNode = initializeNode(newClient);
                        appendToLinkedList(clientList, newNode);
                        Item* newItem = initializeItem(NULL, 0, clientIP, clientPort);

                        pthread_mutex_lock(&(threads->mutexLock));
                        printf("\n  Gained lock to add item.\n");
                        addToRoundBuffer(threadRoundBuffer, newItem, threads);
                        pthread_mutex_unlock(&(threads->mutexLock));
                        pthread_cond_broadcast(&(threads->mutexCond));
                    }
                }
            }
            // Request to return list with pathnames:
            else if(strcmp(requestString, "GET_FILE_LIST") == 0)
            {
                char* IPString = strtok(NULL, " ");
                char* PortString = strtok(NULL, " ");
                uint32_t partnerIP = strtol(IPString, NULL, 16);
                uint16_t partnerPort = strtol(PortString, NULL, 16);
                // partnerPort = ntohs(partnerPort);
                Socket* sendFileListSocket = initializeSocket(partnerPort, partnerIP);
                sendFilesList(sendFileListSocket, dirname, serverSocket->socketSize, address, port);
                close(sendFileListSocket->socket);
            }
            // Get files and add them to the round buffer:
            else if(strcmp(requestString, "FILE_LIST") == 0)
            {
                char* IPString = strtok(NULL, " ");
                char* PortString = strtok(NULL, " ");
                uint32_t partnerIP = strtol(IPString, NULL, 16);
                uint16_t partnerPort = strtol(PortString, NULL, 16);
                // partnerPort = ntohs(partnerPort);
                // partnerIP = ntohl(partnerIP);
                char* numString = strtok(NULL, " ");
                int numOfFiles = atoi(numString);
                for(int i = 0; i < numOfFiles; i++)
                {
                    char* filename = strtok(NULL, " ");
                    char* versionString = strtok(NULL, " ");
                    int version = atoi(versionString);

                    Item* newItem = initializeItem(filename, version, partnerIP, partnerPort);

                    pthread_mutex_lock(&(threads->mutexLock));
                    printf("\n  Gained lock to add item.\n");
                    addToRoundBuffer(threadRoundBuffer, newItem, threads);
                    pthread_mutex_unlock(&(threads->mutexLock));
                    pthread_cond_broadcast(&(threads->mutexCond));
                }
            }
            // Request to return specific file:
            else if(strcmp(requestString, "GET_FILE") == 0)
            {
                char* IPString = strtok(NULL, " ");
                char* PortString = strtok(NULL, " ");
                char* fileToGet = strtok(NULL, " ");
                uint32_t partnerIP = strtol(IPString, NULL, 16);
                uint16_t partnerPort = strtol(PortString, NULL, 16);
                // partnerPort = ntohs(partnerPort);
                char* completeDir = malloc(strlen(dirname) + strlen(fileToGet) + 1);
                strcpy(completeDir, dirname);
                strcat(completeDir, "/");
                strcat(completeDir, fileToGet);
                Socket* sendFileSocket = initializeSocket(partnerPort, partnerIP);
                sendFile(sendFileSocket, completeDir, dirname, serverSocket->socketSize, address, port);
                close(sendFileSocket->socket);
                free(completeDir);
            }
            // Get file data and create file:
            else if(strcmp(requestString, "FILE") == 0)
            {
                // char* IPString = strtok(NULL, " ");
                // char* PortString = strtok(NULL, " ");
                char* fileToGet = strtok(NULL, " ");
                printf("````````FILE TO GET:%s\n", fileToGet);
                char* baseDir = strtok(NULL, " ");
                char* fileSizeString = strtok(NULL, " ");
                // uint32_t partnerIP = strtol(IPString, NULL, 16);
                // uint16_t partnerPort = strtol(PortString, NULL, 16);
                long fileSize = strtol(fileSizeString, NULL, 10);
                createDirectory(dirname, fileToGet+(strlen(baseDir)+1));
                int shift = 22 + strlen(fileToGet) + strlen(baseDir) + strlen(fileSizeString);
                
                // Open file with write permissions, create it if it doesn't exist, empty it if it already exists.
                // Allow the user to read and write that file.
                char* newFile = malloc(strlen(dirname)+strlen(fileToGet));
                strcpy(newFile, dirname);
                strcat(newFile, "/");
                strcat(newFile, fileToGet+(strlen(baseDir)+1));
                printf("````````NEW FILENAME:%s %s %s\n", newFile, fileToGet, fileSizeString);
                int file = open(newFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                char* buffer2 = buffer + shift;

                while(fileSize >= 0)
                {
                    // printf("SHIFT:%d %d %d %d\n", shift, strlen(fileToGet), strlen(baseDir), strlen(fileSizeString));
                    long bufferLen = serverSocket->socketSize - shift;
                    if(fileSize > bufferLen)
                    {
                        if ((write(file, buffer2, bufferLen)) == -1)
                        { 
                            perror ("Error writing to file.");
                            return 1;
                        }
                        // Receive request:
                        if((recv(newSocket->socket, buffer, serverSocket->socketSize, 0)) < 0)
                        {
                            perror("Error in recvfrom.");
                            exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        if ((write(file, buffer2, fileSize)) == -1)
                        { 
                            perror ("Error writing to file.");
                            return 1;
                        }
                    }
                    fileSize -= bufferLen;
                    shift = 0;
                    buffer2 = buffer;
                }
                close(file);
            }
            else if(strcmp(requestString, "USER_ON") == 0)
            {
                char* IPString = strtok(NULL, " ");
                char* PortString = strtok(NULL, " ");
                // Convert them to 32bit and 16bit values:
                uint32_t clientIP = strtol(IPString, NULL, 16);
                // clientIP = ntohl(clientIP);
                uint16_t clientPort = strtol(PortString, NULL, 16);
                // clientPort = ntohs(clientPort);

                // Search for client in clients list:
                int found = checkClientInLinkedList(clientList, clientPort, clientIP);

                // If it doesn't exist, add client to list:
                if(found == 1)
                {
                    // printf("------Adding to rb: %d %d\n", htons(clientPort), htons(port));
                    Client* newClient = initializeClient(clientPort, clientIP);
                    Node* newNode = initializeNode(newClient);
                    appendToLinkedList(clientList, newNode);
                    Item* newItem = initializeItem(NULL, 0, clientIP, clientPort);

                    pthread_mutex_lock(&(threads->mutexLock));
                    printf("\n  Gained lock to add item.\n");
                    addToRoundBuffer(threadRoundBuffer, newItem, threads);
                    pthread_mutex_unlock(&(threads->mutexLock));
                    pthread_cond_broadcast(&(threads->mutexCond));
                }
            }
            close(newSocket->socket);
            bufferCopy = NULL;
        }        
    }

    return 0;
}