#include "requests.h"
#include "threads.h"

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

int sendFilesList(Socket* clientSocket, char* dirname, int size)
{
    int numberOfFiles = 0;

    DIR *directory;
    struct dirent *directory_entry;

    char* firstDirName = malloc(sizeof(char) * strlen(dirname));
    strcpy(firstDirName, dirname);

    const int dirCharsToRemove = strlen(firstDirName) + 1;

    // BFS traversal using a linked list. I prefer it that way, as I have control over
    // the memory being used, unlike the recursive solution.
    Node* firstNode = initializeNode(firstDirName);
    LinkedList* toVisit = initializeLinkedList();
    appendToLinkedList(toVisit, firstNode);

    // Create a linked list with filenames:
    LinkedList* files = initializeLinkedList();

    // While our queue is not empty:
    while(toVisit->head != NULL)
    {
        Node* node = popStart(toVisit);
        char* dirName = (char*)node->item;
        if ((directory = opendir(dirName)) == NULL)
            break;
        // For every subdirectory in a directory:
        while((directory_entry = readdir(directory)) != NULL)
        {
            if(strcmp(directory_entry->d_name, ".") == 0 || strcmp(directory_entry->d_name, "..") == 0)
                continue;
            if(directory_entry->d_type == DT_DIR)
            {
                char* newDirName = malloc(sizeof(char) * (
                    strlen(directory_entry->d_name) + strlen(dirName) + 2));
                strcpy(newDirName, dirName);
                strcat(newDirName, "/");
                strcat(newDirName, directory_entry->d_name);
                // printf("\n  dir: %s\n", newDirName);
                Node* newNode = initializeNode(newDirName);
                appendToLinkedList(toVisit, newNode);
            }
            else
            {
                // Create filename path using directory and filename:
                char* filename = malloc(sizeof(char) * (
                    strlen(directory_entry->d_name) + strlen(dirName) + 2));
                strcpy(filename, dirName);
                strcat(filename, "/");
                strcat(filename, directory_entry->d_name);
                // printf("\n  f: %s\n", filename);
                char* finalFilename = malloc(sizeof(char) * (strlen(filename) - dirCharsToRemove) + 2);
                strcpy(finalFilename, filename+dirCharsToRemove);
                Node* finalFilenameNode = initializeNode(finalFilename);
                appendToLinkedList(files, finalFilenameNode);
                numberOfFiles++;
                free(filename);
            }
        }
        free(node->item);
        free(node);
        closedir(directory);
    }
    free(toVisit);

    // Initialize buffer with request text and number of files:
    char* buffer = malloc(size * sizeof(char));
    strcpy(buffer, "FILE_LIST ");
    char filesNumString[12];
    sprintf(filesNumString, "%d", numberOfFiles);
    strcat(buffer, " ");
    strcat(buffer, filesNumString);

    // Add filenames to buffer:
    while(files->head != NULL)
    {
        Node* node = popStart(files);
        char* fileName = (char*)node->item;
        strcat(buffer, " ");
        strcat(buffer, fileName);
        strcat(buffer, " ");
        strcat(buffer, "1");
        free(node->item);
        free(node);
    }
    free(files);

    printf("Buffer Files: %s\n", buffer);

    return 0;
}