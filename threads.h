#ifndef ERGASIA_3_THREADS_H
#define ERGASIA_3_THREADS_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

#include "requests.h"


typedef struct Item{
    char pathname[128];
    int version;
    u_int32_t IP;
    u_int16_t port;
}Item;

typedef struct roundBuffer{
    Item** items;
    int capacity;
    int currentSize;
    int current;
}roundBuffer;

typedef struct Threadpool{
    pthread_t* threads;
    int numberOfThreads;
    pthread_mutex_t mutexLock;
    pthread_cond_t mutexCond;
    pthread_cond_t mutexCond2;
    int end;
    roundBuffer* buffer;
    uint32_t localIP;
    uint16_t localPort;
    LinkedList* clients;
}Threadpool;

Item* initializeItem(char* pathname, int version, u_int32_t IP, u_int16_t port);
roundBuffer* initializeRoundBuffer(int size);
int addToRoundBuffer(roundBuffer* buffer, Item* item, Threadpool* tPool);
Item* getRoundBufferItem(roundBuffer* buffer);
void printRoundBuffer(roundBuffer* buffer);

Threadpool* initializeThreadpool(int numberOfThreads, roundBuffer* buffer, uint32_t ip, uint16_t port,
    LinkedList* clients);
void* workerThread(void* arg);
#endif //ERGASIA_3_THREADS_H