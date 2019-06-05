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
        uint32_t address = client->clientIP;
        // address = htonl(client->clientIP);
        // Make it a string:
        char* addressString = malloc(8 * sizeof(char));
        sprintf(addressString, "%x", address);
        strcat(buffer, addressString);
        strcat(buffer, " ");

        // Convert ip to network order:
        // uint16_t portBinary = htons(client->clientPort);
        // Make it a string:
        char* portString = malloc(4 * sizeof(char));
        sprintf(portString, "%x", client->clientPort);
        strcat(buffer, portString);
        strcat(buffer, " ");
        node = node->next;
    }
    if(connect(clientSocket->socket, (struct sockaddr*)&clientSocket->socketAddress, sizeof(clientSocket->socketAddress)) < 0)
    {
        perror("1. Error connecting to client.");
        exit(EXIT_FAILURE);
    }
    printf("Buffer: %s Size: %d\n", buffer, size);
    send(clientSocket->socket, buffer, size, 0);
    free(buffer);
    return 0;    
}

int sendFilesList(Socket* clientSocket, char* dirname, int size, uint32_t ip, uint16_t port)
{
    int numberOfFiles = 0;

    DIR *directory;
    struct dirent *directory_entry;

    char* firstDirName = malloc(sizeof(char) * strlen(dirname));
    strcpy(firstDirName, dirname);

    // printf("SENDING FILES LIST: FROM %d TO %d\n", htons(port2), htons(port));

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

    // Initialize buffer with request text, ip, port and number of files:
    char* buffer = malloc(size * sizeof(char));
    strcpy(buffer, "FILE_LIST ");

    char* addressString = malloc(8 * sizeof(char));
    sprintf(addressString, "%x", ip);
    strcat(buffer, addressString);
    strcat(buffer, " ");

    // Convert ip to network order:
    // uint16_t portBinary = htons(port);
    // Make it a string:
    char* portString = malloc(4 * sizeof(char));
    sprintf(portString, "%x", port);
    strcat(buffer, portString);
    strcat(buffer, " ");

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

    // printf("Buffer Files: %d %s\n", size, buffer);
    if(connect(clientSocket->socket, (struct sockaddr*)&clientSocket->socketAddress, sizeof(clientSocket->socketAddress)) < 0)
    {
        perror("2. Error connecting to client.");
        exit(EXIT_FAILURE);
    }
    send(clientSocket->socket, buffer, size, 0);

    return 0;
}

// Transfer give file:
int sendFile(Socket* clientSocket, char* dirname, char* basedir, int size, uint32_t ip, uint16_t port)
{
    // Get filesize:
    struct stat fileStat;
    stat(dirname, &fileStat);
    long contentSize = fileStat.st_size;

    if(connect(clientSocket->socket, (struct sockaddr*)&clientSocket->socketAddress, sizeof(clientSocket->socketAddress)) < 0)
    {
        perror("3. Error connecting to client.");
        exit(EXIT_FAILURE);
    }

    char* buffer = malloc(size * sizeof(char));
    int shift = 5;

    strcpy(buffer, "FILE ");

    char* addressString = malloc(8 * sizeof(char));
    sprintf(addressString, "%x", ip);
    strcat(buffer, addressString);
    strcat(buffer, " ");
    shift += strlen(addressString) + 1;

    char* portString = malloc(4 * sizeof(char));
    sprintf(portString, "%x", port);
    strcat(buffer, portString);
    strcat(buffer, " ");
    shift += strlen(portString) + 1;

    strcat(buffer, dirname);
    strcat(buffer, " ");
    shift += strlen(dirname) + 1;

    strcat(buffer, basedir);
    strcat(buffer, " ");
    shift += strlen(basedir) + 1;

    char filesSizeString[12];
    sprintf(filesSizeString, "%ld", contentSize);
    strcat(buffer, filesSizeString);
    strcat(buffer, " ");
    shift += strlen(filesSizeString) + 1;

    int openFile = open(dirname, O_RDONLY);

    read(openFile, buffer+shift, size-shift);
    send(clientSocket->socket, buffer, size, 0);

    // Get file content:
    bzero(buffer, size);
    int bytes;
    while((bytes = read(openFile, buffer, size)))
    {
        printf("\t\t SENDING FILE DATA\n");
        send(clientSocket->socket, buffer, size, 0);
        bzero(buffer, size);
    }
    close(openFile);
}

// Create directory for new file:
void createDirectory(char* baseDir, char* newDir)
{
    char* fullDir = malloc(strlen(baseDir) + strlen(newDir));
    strcpy(fullDir, baseDir);
    strcat(fullDir, "/");
    char* newDirCopy = malloc(strlen(newDir));
    strcpy(newDirCopy, newDir);
    strcat(fullDir, dirname(newDirCopy));
    // char* fullDir = dirname(newDir);
    printf("\tNEW DIR:%s %s\n", fullDir, baseDir);
    struct stat dirExists;
    // Create dir for file:
    if (!(stat(fullDir, &dirExists) == 0 && S_ISDIR(dirExists.st_mode)))
    {
        mkdir(fullDir, 0777);
    }
    free(fullDir);
    free(newDirCopy);
}

// Send new user notification to clients:
void sendUserOn(LinkedList* clients, char* ip, char* port)
{
    char* message = malloc(22);
    strcpy(message, "USER_ON ");
    strcat(message, ip);
    strcat(message, " ");
    strcat(message, port);
    strcat(message, " ");

    Node* node = clients->head;
    while(node != NULL)
    {
        uint32_t nodeIP = ((Client*)(node->item))->clientIP;
        uint16_t nodePort = ((Client*)(node->item))->clientPort;
        Socket* newSocket = initializeSocket(nodePort, nodeIP);
        if(connect(newSocket->socket, (struct sockaddr*)&newSocket->socketAddress, sizeof(newSocket->socketAddress)) < 0)
        {
            perror("4. Error connecting to client.");
            exit(EXIT_FAILURE);
        }
        send(newSocket->socket, message, 22, 0);
        shutdown(newSocket->socket, SHUT_RDWR);
        close(newSocket->socket);
        node = node->next;
    }
}