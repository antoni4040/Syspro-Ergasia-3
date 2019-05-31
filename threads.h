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
    int end;
    roundBuffer* buffer;
}Threadpool;

Item* initializeItem(char* pathname, int version, u_int32_t IP, u_int16_t port);
roundBuffer* initializeRoundBuffer(int size);
int addToRoundBuffer(roundBuffer* buffer, Item* item);
Item* getRoundBufferItem(roundBuffer* buffer);

Threadpool* initializeThreadpool(int numberOfThreads, roundBuffer* buffer);
void* workerThread(void* arg);
#endif //ERGASIA_3_THREADS_H