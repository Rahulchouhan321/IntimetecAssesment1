#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 2003

typedef struct Node 
{
    int key;
    char value[100];
    struct Node *prev, *next;
} Node;

typedef struct 
{
    int capacity;
    int size;
    Node *head;
    Node *tail;
    Node *hash[HASH_SIZE];
} LRUCache;

int getInt() 
{
    int intdata;
    char chardata;
    while (1) {
        if (scanf("%d", &intdata) == 1) {
            while ((chardata = getchar()) != '\n' && chardata != EOF);
            return intdata;
        }
        printf("Invalid integer! Try again.\n");
        while ((chardata = getchar()) != '\n' && chardata != EOF);
    }
}

void getString(char *buffer, int size) {
    char charData;
    while (1) {
        if (scanf("%99s", buffer) == 1) {
            while ((charData = getchar()) != '\n' && charData != EOF);
            return;
        }
        printf("Invalid string! Try again.\n");
        while ((charData = getchar()) != '\n' && charData != EOF);
    }
}

void getCommand(char *cmdPtr) {
    char charData;
    while (1) {
        if (scanf("%49s", cmdPtr) == 1) {
            while ((charData = getchar()) != '\n' && charData != EOF);
            return;
        }
        printf("Invalid command! Try again.\n");
        while ((charData = getchar()) != '\n' && charData != EOF);
    }
}

int hashFunction(int key) {
    return (key % HASH_SIZE + HASH_SIZE) % HASH_SIZE;
}

Node* createNode(int key, const char *value) {
    Node *n = malloc(sizeof(Node));
    n->key = key;
    strcpy(n->value, value);
    n->prev = n->next = NULL;
    return n;
}

LRUCache* createCache(int capacity) {
    LRUCache *cache = malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;
    for(int i=0;i<HASH_SIZE;i++) cache->hash[i]=NULL;
    return cache;
}

void addToHead(LRUCache *cache, Node *node) {
    node->prev = NULL;
    node->next = cache->head;
    if(cache->head) cache->head->prev = node;
    cache->head = node;
    if(cache->tail == NULL) cache->tail = node;
}

void removeNode(LRUCache *cache, Node *node) {
    if(node->prev) node->prev->next = node->next;
    else cache->head = node->next;
    if(node->next) node->next->prev = node->prev;
    else cache->tail = node->prev;
}

void moveToHead(LRUCache *cache, Node *node) {
    removeNode(cache, node);
    addToHead(cache, node);
}

void removeTail(LRUCache *cache) {
    Node *lru = cache->tail;
    int index = hashFunction(lru->key);
    cache->hash[index] = NULL;
    removeNode(cache, lru);
    free(lru);
    cache->size--;
}

char* get(LRUCache *cache, int key) {
    int index = hashFunction(key);
    Node *node = cache->hash[index];
    if(node == NULL) return NULL;
    moveToHead(cache, node);
    return node->value;
}

void put(LRUCache *cache, int key, const char *value) {
    int index = hashFunction(key);
    Node *node = cache->hash[index];

    if(node != NULL) {
        strcpy(node->value, value);
        moveToHead(cache, node);
        return;
    }

    Node *newNode = createNode(key, value);
    cache->hash[index] = newNode;
    addToHead(cache, newNode);
    cache->size++;

    if(cache->size > cache->capacity) removeTail(cache);
}

int main() {
    LRUCache *cache = NULL;
    char command[50], value[100];

    while(1) {
        getCommand(command);

        if(strcmp(command, "createCache") == 0) {
            int cap = getInt();
            cache = createCache(cap);
        }
        else if(strcmp(command, "put") == 0) {
            int key = getInt();
            getString(value, 100);
            put(cache, key, value);
        }
        else if(strcmp(command, "get") == 0) {
            int key = getInt();
            char *res = get(cache, key);
            if(res) printf("%s\n", res);
            else printf("NULL\n");
        }
        else if(strcmp(command, "exit") == 0) {
            break;
        }
        else {
            printf("Invalid command! Try again.\n");
        }
    }

    return 0;
}
