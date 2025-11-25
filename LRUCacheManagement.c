#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 2003

typedef struct Node 
{
    int key;
    char value[100];
    struct Node *prevPtr, *nextPtr;
} Node;

typedef struct HashEntry 
{
    int key;
    Node *nodePtr;
    struct HashEntry *nextPtr;
} HashEntry;

typedef struct 
{
    int capacity;
    int size;
    Node *headPtr;
    Node *tailPtr;
    HashEntry *hashTable[HASH_SIZE];
} LRUCache;

int hashFunction(int key) 
{
    return (key % HASH_SIZE + HASH_SIZE) % HASH_SIZE;
}

HashEntry* hashmapGet(LRUCache *cachePtr, int key) 
{
    int index = hashFunction(key);
    HashEntry *entryPtr = cachePtr->hashTable[index];

    while (entryPtr) 
    {
        if (entryPtr->key == key)
            return entryPtr;

        entryPtr = entryPtr->nextPtr;
    }
    return NULL;
}

void hashmapPut(LRUCache *cachePtr, int key, Node *nodePtr) 
{
    int index = hashFunction(key);
    HashEntry *entryPtr = cachePtr->hashTable[index];

    while (entryPtr) 
    {
        if (entryPtr->key == key) 
        {
            entryPtr->nodePtr = nodePtr;
            return;
        }
        entryPtr = entryPtr->nextPtr;
    }

    HashEntry *newEntryPtr = malloc(sizeof(HashEntry));
    newEntryPtr->key = key;
    newEntryPtr->nodePtr = nodePtr;
    newEntryPtr->nextPtr = cachePtr->hashTable[index];
    cachePtr->hashTable[index] = newEntryPtr;
}

void hashmapRemove(LRUCache *cachePtr, int key) 
{
    int index = hashFunction(key);
    HashEntry *entryPtr = cachePtr->hashTable[index];
    HashEntry *prevPtr = NULL;

    while (entryPtr) 
    {
        if (entryPtr->key == key) 
        {
            if (prevPtr) prevPtr->nextPtr = entryPtr->nextPtr;
            else cachePtr->hashTable[index] = entryPtr->nextPtr;

            free(entryPtr);
            return;
        }
        prevPtr = entryPtr;
        entryPtr = entryPtr->nextPtr;
    }
}

Node* createNode(int key, const char *value) 
{
    Node *newNodePtr = malloc(sizeof(Node));
    newNodePtr->key = key;
    strcpy(newNodePtr->value, value);
    newNodePtr->prevPtr = newNodePtr->nextPtr = NULL;
    return newNodePtr;
}

LRUCache* createCache(int capacity) 
{
    LRUCache *cachePtr = malloc(sizeof(LRUCache));
    cachePtr->capacity = capacity;
    cachePtr->size = 0;
    cachePtr->headPtr = NULL;
    cachePtr->tailPtr = NULL;

    for (int i = 0; i < HASH_SIZE; i++)
        cachePtr->hashTable[i] = NULL;

    return cachePtr;
}

void addToHead(LRUCache *cachePtr, Node *nodePtr) 
{
    nodePtr->prevPtr = NULL;
    nodePtr->nextPtr = cachePtr->headPtr;

    if (cachePtr->headPtr)
        cachePtr->headPtr->prevPtr = nodePtr;

    cachePtr->headPtr = nodePtr;

    if (cachePtr->tailPtr == NULL)
        cachePtr->tailPtr = nodePtr;
}

void removeNode(LRUCache *cachePtr, Node *nodePtr) 
{
    if (nodePtr->prevPtr)
        nodePtr->prevPtr->nextPtr = nodePtr->nextPtr;
    else
        cachePtr->headPtr = nodePtr->nextPtr;

    if (nodePtr->nextPtr)
        nodePtr->nextPtr->prevPtr = nodePtr->prevPtr;
    else
        cachePtr->tailPtr = nodePtr->prevPtr;

    nodePtr->prevPtr = NULL;
    nodePtr->nextPtr = NULL;
}

void moveToHead(LRUCache *cachePtr, Node *nodePtr) 
{
    removeNode(cachePtr, nodePtr);
    addToHead(cachePtr, nodePtr);
}

void removeTail(LRUCache *cachePtr) 
{
    Node *lruNodePtr = cachePtr->tailPtr;

    hashmapRemove(cachePtr, lruNodePtr->key);
    removeNode(cachePtr, lruNodePtr);
    free(lruNodePtr);
    cachePtr->size--;
}

char* get(LRUCache *cachePtr, int key) 
{
    HashEntry *entryPtr = hashmapGet(cachePtr, key);

    if (entryPtr == NULL)
        return NULL;

    moveToHead(cachePtr, entryPtr->nodePtr);
    return entryPtr->nodePtr->value;
}

void put(LRUCache *cachePtr, int key, const char *value) 
{
    HashEntry *entryPtr = hashmapGet(cachePtr, key);
    if (entryPtr != NULL) 
    {
        strcpy(entryPtr->nodePtr->value, value);
        moveToHead(cachePtr, entryPtr->nodePtr);
        return;
    }

    Node *newNodePtr = createNode(key, value);
    addToHead(cachePtr, newNodePtr);
    hashmapPut(cachePtr, key, newNodePtr);
    cachePtr->size++;

    if (cachePtr->size > cachePtr->capacity)
        removeTail(cachePtr);
}
void readLine(char *buffer, int size) 
{
    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';  
}

int main() 
{
    LRUCache *cachePtr = NULL;
    char inputLine[200];

    while (1) 
    {
        readLine(inputLine, 200);
        char command[50];
        int key, cap;
        char value[100];
        if (sscanf(inputLine, "%49s %d", command, &cap) == 2 &&
            strcmp(command, "createCache") == 0) 
        {
            cachePtr = createCache(cap);
            printf("LRU Cache of capacity %d created.\n", cap);
        }
        else if (sscanf(inputLine, "%49s %d %99s", command, &key, value) == 3 &&
                                                 strcmp(command, "put") == 0)
        {
            put(cachePtr, key, value);
        }
        else if(sscanf(inputLine, "%49s %d", command, &key) == 2 &&
                                    strcmp(command, "get") == 0)
        {
            char *res = get(cachePtr, key);
            printf("%s\n", res ? res : "NULL");
        }
        else if (strcmp(inputLine, "exit") == 0) 
        {
            break;
        }
        else 
        {
            printf("Invalid command! Try again.\n");
        }
    }

    return 0;
}

