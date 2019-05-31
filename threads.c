#include "threads.h"

// Initialize a round buffer item with given parameters:
Item* initializeItem(char* pathname, int version, u_int32_t IP, u_int16_t port)
{
    Item* newItem = malloc(sizeof(Item));
    if(pathname != NULL)
        strcpy(newItem->pathname, pathname);
    newItem->version = version;
    newItem->IP = IP;
    newItem->port = port;
    return newItem;
}

// Initialize round buffer with given size:
roundBuffer* initializeRoundBuffer(int size)
{
    roundBuffer* newRoundBuffer = malloc(sizeof(roundBuffer));
    newRoundBuffer->items = malloc(sizeof(Item*) * size); 
    newRoundBuffer->capacity = size;
    newRoundBuffer->current = 0;
    newRoundBuffer->currentSize = 0;
    return newRoundBuffer;
}

// Add item to round buffer if a place is available:
int addToRoundBuffer(roundBuffer* buffer, Item* item)
{
    if(buffer->capacity == buffer->currentSize)
    {
        return 1;
    }
    else
    {
        for(int i = 0; i < buffer->capacity; i++)
        {
            if(buffer->items[i] == NULL)
            {
                buffer->items[i] = item;
                buffer->currentSize++;
                break;
            }
        }
    }
    return 0;
}

// Get first available round buffer item:
Item* getRoundBufferItem(roundBuffer* buffer)
{
    Item* itemToReturn = NULL;
    for(int i = 0; i < buffer->capacity; i++)
    {
        if(buffer->items[i] != NULL)
        {
            itemToReturn = buffer->items[i];
            buffer->items[i] = NULL;
            break;
        }
    }
    return itemToReturn;
}

Threadpool* initializeThreadpool(int numberOfThreads, roundBuffer* buffer)
{
    // Int new threadpool:
    Threadpool* newThreadpool = malloc(sizeof(Threadpool));
    newThreadpool->numberOfThreads = numberOfThreads;
    newThreadpool->threads = malloc(numberOfThreads * sizeof(pthread_t));
    newThreadpool->end = 0;
    newThreadpool->buffer = buffer;

    // Init mutex:
    pthread_mutex_init(&(newThreadpool->mutexLock), NULL);
    pthread_cond_init(&(newThreadpool->mutexCond), NULL);

    // Create threads in threadpool:
    for(int i=0; i<numberOfThreads; i++){
        pthread_create(&newThreadpool->threads[i], NULL, workerThread, (void*)newThreadpool);
        pthread_detach(newThreadpool->threads[i]);
    }

    return newThreadpool;
}


void* workerThread(void* arg)
{
    // Get threadpool from void pointer argument:
    Threadpool* tPool = (Threadpool*)arg;
    
    // While threadpool still alive:
    while(tPool->end == 0)
    {
        // Try to get lock:
        pthread_mutex_lock(&(tPool->mutexLock));

        // Wait until round buffer has available items: 
        pthread_cond_wait(&(tPool->mutexCond), &(tPool->mutexLock));
        printf("\n Unlock to get item.\n");

        // If threadpool dead, get over it:
        if(tPool->end == 1){
            pthread_mutex_unlock(&(tPool->mutexLock));
            break;
        }

        // Get an item from round buffer:
        Item* item = getRoundBufferItem(tPool->buffer);
        // printf("New item: %d %d\n", item->IP, item->port);

        // If there is only ip and port, ask for files:
        if(item->version == 0)
        {
            Socket* newSocket = initializeSocket(item->port, item->IP);
            // Initialize connection:
            if(connect(newSocket->socket, (struct sockaddr*)&newSocket->socketAddress, sizeof(newSocket->socketAddress)) < 0)
            {
                perror("Error connecting to gain files.");
                exit(EXIT_FAILURE);
            }
            char filesRequest[1024];
            strcpy(filesRequest, "GET_FILE_LIST ");

            char* addressString = malloc(8 * sizeof(char));
            uint32_t address = htonl(item->IP);
            sprintf(addressString, "%x", address);

            strcat(filesRequest, addressString);
            strcat(filesRequest, " ");

            char* portString = malloc(4 * sizeof(char));
            uint16_t port = htons(item->port);
            sprintf(portString, "%x", port);

            strcat(filesRequest, portString);
            strcat(filesRequest, " ");

            printf("%s\n", filesRequest);
            send(newSocket->socket, filesRequest, 1024, 0);
            close(newSocket->socket);
        }

        pthread_mutex_unlock(&(tPool->mutexLock));
    }
    return 0;
}