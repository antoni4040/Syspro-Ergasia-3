// Created by Antonis Karvelas.
// Here lies the generic linked list, plus a few client specific things:
#ifndef ERGASIA_1_LINKEDLIST_H
#define ERGASIA_1_LINKEDLIST_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct client
{
    int16_t clientPort;
    int32_t clientIP;
} Client;

typedef struct Node
{
    void* item;
    struct Node* next;
    struct Node* previous;
} Node;

typedef struct LinkedList
{
    Node* head;
    Node* tail;
} LinkedList;

// Generic part:
Node* initializeNode(void* item);
LinkedList* initializeLinkedList();
void appendToLinkedList(LinkedList* linkedList, Node* node);
void freeLinkedList(LinkedList* linkedList);
Node* popStart(LinkedList* linkedList);
int linkedListSize(LinkedList* linkedList);

// Client part:
Client* initializeClient(int16_t port, int32_t clientIP);
int checkClientInLinkedList(LinkedList* linkedList, int16_t port, int32_t ip);
#endif //ERGASIA_1_LINKEDLIST_H