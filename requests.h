#ifndef ERGASIA_3_REQUESTS_H
#define ERGASIA_3_REQUESTS_H
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <libgen.h>

#include <unistd.h>

#include <dirent.h>

#include "linked_list.h"

typedef struct Socket{
    int socket;
    struct sockaddr_in socketAddress;
    int socketSize;
}Socket;

Socket* initializeSocket(uint16_t port, uint32_t ip);
int sendClientList(Socket* clientSocket, LinkedList* clientList, int size);
int sendFilesList(Socket* clientSocket, char* dirname, int size, uint32_t ip, uint16_t port);
int sendFile(Socket* clientSocket, char* dirname, char* basedir, int size, uint32_t ip, uint16_t port);
void createDirectory(char* baseDir, char* newDir);
void sendUserOn(LinkedList* clients, char* ip, char* port);
#endif //ERGASIA_3_REQUESTS_H