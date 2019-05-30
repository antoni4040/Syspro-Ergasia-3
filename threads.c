#include "threads.h"

Threadpool* initializeThreadpool(int numberOfThreads)
{
    // Int new threadpool:
    Threadpool* newThreadpool = malloc(sizeof(Threadpool));
    newThreadpool->numberOfThreads = numberOfThreads;
    newThreadpool->threads = malloc(numberOfThreads * sizeof(pthread_t));

    // Init mutex:
    pthread_mutex_init(&(newThreadpool->mutexLock), NULL);
    pthread_cond_init(&(newThreadpool->mutexCond), NULL);

    // Create threads in threadpool:
    for(int i=0; i<numberOfThreads; i++){
        pthread_create(&newThreadpool->threads[i], NULL, NULL, (void*)NULL);
        pthread_detach(newThreadpool->threads[i]);
    }

    return newThreadpool;
}