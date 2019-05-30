#include "requests.h"

// Initialize a socket with given port and ip:
Socket* initializeSocket(uint16_t port, uint32_t ip)
{
    Socket* newSocket = malloc(sizeof(Socket));
    if((newSocket->socket = socket(AF_INET , SOCK_STREAM , 0)) < 0)
    {
        perror("Error creating socket.");
        exit(EXIT_FAILURE);
    }

    // Set socket options:
    unsigned int socklen = sizeof(int);
    getsockopt(newSocket->socket, SOL_SOCKET, SO_RCVBUF, &newSocket->socketSize, &socklen);
    setsockopt(newSocket->socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    newSocket->socketAddress.sin_family = AF_INET;
    newSocket->socketAddress.sin_addr.s_addr = htonl(ip);
    newSocket->socketAddress.sin_port = htons(port);
    return newSocket;
}

int sendClientList(Socket* clientSocket, LinkedList* clientList, int size)
{
    char* buffer = malloc(size * sizeof(char));
    strcpy(buffer, "CLIENT_LIST ");
    int clientListSize = linkedListSize(clientList);
    char clientListSizeString[12];
    sprintf(clientListSizeString, "%d", clientListSize);
    strcat(buffer, clientListSizeString);
    strcat(buffer, " ");

    Node* node = clientList->head;
    while(node != NULL)
    {
        Client* client = (Client*)(node->item);
        // Convert ip to network order:
        uint32_t address;
        address = htonl(client->clientIP);
        // Make it a string:
        char* addressString = malloc(8 * sizeof(char));
        sprintf(addressString, "%x", address);
        strcat(buffer, addressString);
        strcat(buffer, " ");

        // Convert ip to network order:
        uint16_t portBinary = htons(client->clientPort);
        // Make it a string:
        char* portString = malloc(4 * sizeof(char));
        sprintf(portString, "%x", portBinary);
        strcat(buffer, portString);
        strcat(buffer, " ");
        node = node->next;
    }
    printf("client port aaaa: %d\n", clientSocket->socketAddress.sin_port);

    if(connect(clientSocket->socket, (struct sockaddr*)&clientSocket->socketAddress, sizeof(clientSocket->socketAddress)) < 0)
    {
        perror("Error connecting to client.");
        exit(EXIT_FAILURE);
    }
    printf("Buffer: %s Size: %d\n", buffer, size);
    send(clientSocket->socket, buffer, size, 0);
    free(buffer);
    return 0;    
}