// Created by Antonis Karvelas.
// Here lies the generic linked list, plus a few client specific things:
#include "linked_list.h"

// Generic part:

// Create a new linked list node with a given item:
Node* initializeNode(void* item)
{
    Node* newNode = malloc(sizeof(Node));
    newNode->item = item;
    newNode->next = NULL;
    newNode->previous = NULL;
    return newNode;
}

// Create a new empty linked list:
LinkedList* initializeLinkedList()
{
    LinkedList* newLinkedList = malloc(sizeof(LinkedList));
    newLinkedList->head = NULL;
    newLinkedList->tail = NULL;
    return newLinkedList;
}

// Append item to a linked list:
void appendToLinkedList(LinkedList* linkedList, Node* node)
{
    if(linkedList->head == NULL)
    {
        linkedList->head = node;
        linkedList->tail = node;
    }
    else
    {
        Node* previousTail = linkedList->tail;
        linkedList->tail->next = node;
        linkedList->tail = node;
        linkedList->tail->previous = previousTail;
    }
}

// Pop a node from the beginning of the list:
Node* popStart(LinkedList* linkedList)
{
    Node* node = linkedList->head;
    linkedList->head = linkedList->head->next;
    if(linkedList->head == NULL)
        linkedList->tail = NULL;
    return node;
}

// Deallocate memory of a linked list:
void freeLinkedList(LinkedList* linkedList)
{
    Node* node;
    while((node = linkedList->head) != NULL)
    {
        linkedList->head = linkedList->head->next;
        free(node);
    }
    free(linkedList);
}

// Return size of a linked list:
int linkedListSize(LinkedList* linkedList)
{
    int size = 0;
    Node* node = linkedList->head;
    while(node != NULL)
    {
        size++;
        node = node->next;
    }
    return size;
}

// Client part:

// Initialize a client struct to put in a linked list node:
Client* initializeClient(int16_t port, int32_t ip)
{
    Client* newClient = malloc(sizeof(Client));
    newClient->clientPort = port;
    newClient->clientIP = ip;
    return newClient;
}

// Check if a client ip and port already exists in the clients linked list:
int checkClientInLinkedList(LinkedList* linkedList, int16_t port, int32_t ip)
{
    int idFound = 1;
    Node* node = linkedList->head;
    while(node != NULL)
    {
        printf("~~~~ Comparing: %d %d\n", htons(((Client*)(node->item))->clientPort), htons(port));
        if(((Client*)(node->item))->clientIP == ip && 
            ((Client*)(node->item))->clientPort == port)
        {
            idFound = 0;
            break;
        }
        node = node->next;
    }
    return idFound;
}