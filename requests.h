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
int sendClientList(int clientSocket, LinkedList* clientList, int size);

#endif //ERGASIA_3_REQUESTS_H