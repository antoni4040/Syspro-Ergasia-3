#ifndef ERGASIA_3_THREADS_H
#define ERGASIA_3_THREADS_H

#include <pthread.h>

typedef struct Threadpool{
    pthread_t* threads;
    int numberOfThreads;
    pthread_mutex_t mutexLock;
    pthread_cond_t mutexCond;
}Threadpool;

#endif //ERGASIA_3_THREADS_H