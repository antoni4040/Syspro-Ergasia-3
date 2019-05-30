#ifndef ERGASIA_3_REQUESTS_H
#define ERGASIA_3_REQUESTS_H
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

typedef struct Socket{
    int socket;
    struct sockaddr_in socketAddress;
    int socketSize;
}Socket;

Socket* initializeSocket(uint16_t port, uint32_t ip);
int sendClientList(Socket* clientSocket, LinkedList* clientList, int size);
#endif //ERGASIA_3_REQUESTS_H