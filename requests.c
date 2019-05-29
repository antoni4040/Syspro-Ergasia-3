#include "requests.h"

int sendClientList(int clientSocket, LinkedList* clientList, int size)
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
    printf("Buffer: %s\n", buffer);
    return 0;
}