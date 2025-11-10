#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define DEFAULT_NUM_BLOCKS 10
#define MAX_NUM_BLOCKS 50
#define MAX_NAME_LEN 50
#define MAX_FILE_BLOCKS 50 

static unsigned char **sDiskMemory = NULL; 
static int sNumberOfBlocks = 0;

typedef struct FreeBlock 
{
    int index;                  
    struct FreeBlock *prev;    
    struct FreeBlock *next;
} FreeBlock;

static FreeBlock *sFreeBlockHead = NULL; 
static FreeBlock *sFreeBlockTail = NULL; 
static int sFreeCount = 0;        


typedef struct FileNode 
{
    char name[MAX_NAME_LEN + 1];
    bool isDir;
    size_t sizeBytes;              
    int blockCount;                
    int blockPointers[MAX_FILE_BLOCKS]; 
    struct FileNode *parent;       
    struct FileNode *next;         
    struct FileNode *prev;         
    struct FileNode *childHead;    
} FileNode;

static FileNode *sRootNode = NULL;    
static FileNode *sCmdNode  = NULL;    


static void *myMalloc(size_t n) 
{
    void *p = malloc(n);
    if (!p) 
    { 
        fprintf(stderr, "Out of memory\n"); exit(1); 
    }
    return p;
}

static FreeBlock* makeFreeBlockNode(int idx) 
{
    FreeBlock *newNode = (FreeBlock*)myMalloc(sizeof(FreeBlock));
    newNode->index = idx;
    newNode->prev = newNode->next = NULL;
    return newNode;
}

static FileNode* makeNode(const char *fileName, bool isDir) 
{
    FileNode *newNode = (FileNode*)myMalloc(sizeof(FileNode));
    strncpy(newNode->name, fileName, MAX_NAME_LEN);
    newNode->name[MAX_NAME_LEN] = '\0';
    newNode->isDir = isDir;
    newNode->sizeBytes = 0;
    newNode->blockCount = 0;
    for (int i=0;i<MAX_FILE_BLOCKS;++i) newNode->blockPointers[i] = -1;
    newNode->parent = NULL;
    newNode->next = newNode->prev = NULL;
    newNode->childHead = NULL;
    return newNode;
}

static void freeListInit(int numBlocks) 
{
    sFreeBlockHead = sFreeBlockTail = NULL;
    sFreeCount = 0;
    for (int i = 0; i < numBlocks; ++i) 
    {
        FreeBlock *newNode = makeFreeBlockNode(i);
        if (!sFreeBlockTail) 
        {
            sFreeBlockHead = sFreeBlockTail = newNode;
        } 
        else
        {
            newNode->prev = sFreeBlockTail;
            sFreeBlockTail->next = newNode;
            sFreeBlockTail = newNode;
        }
        sFreeCount++;
    }
}

static int freeListPopHead(void) 
{
    if (!sFreeBlockHead) return -1;
    FreeBlock *tempPtr = sFreeBlockHead;
    int idx = tempPtr->index;
    sFreeBlockHead = tempPtr->next;
    if (sFreeBlockHead) sFreeBlockHead->prev = NULL; else sFreeBlockTail = NULL;
    free(tempPtr);
    sFreeCount--;
    return idx;
}

static void freeListAppendTail(int idx) 
{
    FreeBlock *tempPtr = makeFreeBlockNode(idx);
    if (!sFreeBlockTail) 
    {
        sFreeBlockHead = sFreeBlockTail = tempPtr;
    } else 
    {
        tempPtr->prev = sFreeBlockTail;
        sFreeBlockTail->next = tempPtr;
        sFreeBlockTail = tempPtr;
    }
    sFreeCount++;
}

static void insertChild(FileNode *parent, FileNode *child) 
{
    child->parent = parent;
    child->next = child->prev = NULL;
    if (!parent->childHead) 
    {
        parent->childHead = child;
        child->next = child->prev = child; 
        return;
    }
    FileNode *head = parent->childHead;
    FileNode *tail = head->prev; 
    child->next = head;
    child->prev = tail;
    tail->next = child;
    head->prev = child;
}

static int removeChild(FileNode *parent, FileNode *child) 
{
    if (!parent || !parent->childHead || !child) return 0;
    FileNode *head = parent->childHead;
    if (head == child && head->next == head) 
    {
        parent->childHead = NULL;
        child->next = child->prev = NULL;
        return 1;
    }
    FileNode *cur = head;
    do 
    {
        if (cur == child) 
        {
            FileNode *p = cur->prev;
            FileNode *n = cur->next;
            p->next = n; n->prev = p;
            if (parent->childHead == cur) parent->childHead = n; // move head
            cur->next = cur->prev = NULL;
            return 1;
        }
        cur = cur->next;
    } while (cur != head);
    return 0; 
}

static FileNode* findChild(FileNode *dir, const char *name) 
{
    if (!dir || !dir->childHead) return NULL;
    FileNode *headPtr = dir->childHead;
    FileNode *curPtr = headPtr;
    do 
    {
        if (strcmp(curPtr->name, name) == 0) 
            return curPtr;
        curPtr = curPtr->next;
    } while (curPtr != headPtr);
    return NULL;
}

static void initDiskAndVfs(int numBlocks) 
{
    if (numBlocks < 1) numBlocks = 1;
    if (numBlocks > MAX_NUM_BLOCKS) numBlocks = MAX_NUM_BLOCKS;
    sNumberOfBlocks = numBlocks;
    sDiskMemory = (unsigned char**)myMalloc(sizeof(unsigned char*) * sNumberOfBlocks);
    for (int i=0;i<sNumberOfBlocks;++i) 
    {
        sDiskMemory[i] = (unsigned char*)myMalloc(BLOCK_SIZE);
        memset(sDiskMemory[i], 0, BLOCK_SIZE);
    }
    freeListInit(sNumberOfBlocks);
    sRootNode = makeNode("/", true);
    sRootNode->parent = NULL;
    sCmdNode = sRootNode;
}

static void freeNodeRecursive(FileNode *node) 
{
    if (!node) return;
    if (node->isDir) 
    {
        FileNode *headPtr = node->childHead;
        if (headPtr) 
        {
            FileNode *curPtr = headPtr;
            do 
            {
                FileNode *nextPtr = curPtr->next;
                freeNodeRecursive(curPtr);
                curPtr = nextPtr;
            } while (curPtr != headPtr);
        }
    } 
    else 
    {
        for (int i=0;i<node->blockCount;++i) 
        {
            int idx = node->blockPointers[i];
            if (idx >= 0) freeListAppendTail(idx);
        }
    }
    free(node);
}

static void freeVFS(void) 
{
    freeNodeRecursive(sRootNode);
    sRootNode = sCmdNode = NULL;
    while (sFreeBlockHead) 
    {
        FreeBlock *n = sFreeBlockHead;
        sFreeBlockHead = n->next;
        free(n);
    }
    sFreeBlockTail = NULL;
    if (sDiskMemory) 
    {
        for (int i=0;i<sNumberOfBlocks;++i) free(sDiskMemory[i]);
        free(sDiskMemory);
        sDiskMemory = NULL;
    }
    sNumberOfBlocks = 0;
    sFreeCount = 0;
}

static int allocateBlocks(int needed, int *outIdx ) 
{
    for (int i=0;i<needed;++i) 
    {
        int idx = freeListPopHead();
        if (idx < 0) 
        {
            for (int j=0;j<i;++j) freeListAppendTail(outIdx[j]);
            return 0;
        }
        outIdx[i] = idx;
    }
    return 1;
}

static void writeIntoBlocks(const unsigned char *data, size_t len, const int *blocks, int blockCount) 
{
    size_t offset = 0;
    for (int i=0;i<blockCount;++i) 
    {
        size_t tocopy = (len - offset > BLOCK_SIZE) ? BLOCK_SIZE : (len - offset);
        if (tocopy > 0) memcpy(sDiskMemory[blocks[i]], data + offset, tocopy);
        if (tocopy < BLOCK_SIZE) memset(sDiskMemory[blocks[i]] + tocopy, 0, BLOCK_SIZE - tocopy);
        offset += tocopy;
    }
}

static int cmd_create(const char *name) 
{
    if (!name || !*name) 
    { 
        printf("Invalid name\n");
        return 0; 
    }
    if (strlen(name) > MAX_NAME_LEN) 
    { 
        printf("Name too long\n"); 
        return 0; 
    }
    if (findChild(sCmdNode, name)) 
    { 
        printf("Entry exists\n"); 
        return 0; 
    }
    FileNode *filePtr = makeNode(name, false);
    insertChild(sCmdNode, filePtr);
    printf("File '%s' created successfully.\n", name);
    return 1;
}

static int cmd_mkdir(const char *name) 
{
    if (!name || !*name)
    { 
        printf("Invalid name\n"); 
        return 0; 
    }
    if (strlen(name) > MAX_NAME_LEN) 
    { 
        printf("Name too long\n"); 
        return 0; 
    }
    if (findChild(sCmdNode, name)) 
    { 
        printf("Entry exists\n"); 
        return 0; 
    }
    FileNode *dirPtr = makeNode(name, true);
    insertChild(sCmdNode, dirPtr);
    printf("Directory '%s' created successfully.\n", name);
    return 1;
}

static int cmdWrite(const char *name, const char *payload) 
{
    FileNode *tempPtr = findChild(sCmdNode, name);
    if (!tempPtr) 
    {   
        printf("File not found\n"); 
        return 0; 
    }
    if (tempPtr->isDir) 
    { 
        printf("Cannot write to a directory\n"); 
        return 0; 
    }
    size_t len = payload ? strlen(payload) : 0;
    for (int i=0;i<tempPtr->blockCount;++i) 
    {
        if (tempPtr->blockPointers[i] >= 0) 
        {
            freeListAppendTail(tempPtr->blockPointers[i]);
        }
    }
    tempPtr->blockCount = 0;
    if (len == 0) 
    { 
        tempPtr->sizeBytes = 0; 
        printf("Data written successfully (size=0 bytes).\n"); 
        return 1; 
    }
    int needed = (int)((len + BLOCK_SIZE - 1) / BLOCK_SIZE);
    if (needed > MAX_FILE_BLOCKS) 
    { 
        printf("File too large (exceeds block pointer capacity)\n"); 
        return 0; 
    }
    int *needPtr = (int*)myMalloc(sizeof(int)*needed);
    if (!allocateBlocks(needed, needPtr)) 
    {
        free(needPtr);
        printf("Disk full: not enough free blocks.\n");
        return 0;
    }
    for (int i=0;i<needed;++i) tempPtr->blockPointers[i] = needPtr[i];
    tempPtr->blockCount = needed;
    tempPtr->sizeBytes = len;
    writeIntoBlocks((const unsigned char*)payload, len, tempPtr->blockPointers, tempPtr->blockCount);
    printf("Data written successfully (size=%zu bytes).\n", len);
    free(needPtr);
    return 1;
}

static int cmdRead(const char *name) 
{
    FileNode *childNode = findChild(sCmdNode, name);
    if (!childNode)
    { 
        printf("File not found\n"); 
        return 0; 
    }
    if (childNode->isDir) 
    { 
        printf("Cannot read a directory\n"); 
        return 0; 
    }
    size_t remaining = childNode->sizeBytes;
    for (int i=0;i<childNode->blockCount && remaining>0;++i) 
    {
        size_t toread = remaining > BLOCK_SIZE ? BLOCK_SIZE : remaining;
        fwrite(sDiskMemory[childNode->blockPointers[i]], 1, toread, stdout);
        remaining -= toread;
    }
    if (childNode->sizeBytes > 0) putchar('\n');
    return 1;
}

static int cmdDelete(const char *name) {
    FileNode *ChildPtr = findChild(sCmdNode, name);
    if (!ChildPtr) { printf("File not found\n"); return 0; }
    if (ChildPtr->isDir) { printf("Use rmdir for directories\n"); return 0; }
    removeChild(sCmdNode, ChildPtr);
    for (int i=0;i<ChildPtr->blockCount;++i) 
       if (ChildPtr->blockPointers[i] >= 0) 
          freeListAppendTail(ChildPtr->blockPointers[i]);
    free(ChildPtr);
    printf("File deleted successfully.\n");
    return 1;
}

static int cmdRmdir(const char *name) 
{
    FileNode *n = findChild(sCmdNode, name);
    if (!n) { printf("Directory not found\n"); return 0; }
    if (!n->isDir) { printf("Not a directory\n"); return 0; }
    if (n->childHead) { printf("Directory not empty\n"); return 0; }
    removeChild(sCmdNode, n);
    free(n);
    printf("Directory removed successfully.\n");
    return 1;
}

static void cmdLs(void) {
    if (!sCmdNode->childHead) { printf("(empty)\n"); return; }
    FileNode *head = sCmdNode->childHead;
    FileNode *cur = head;
    do {
        if (cur->isDir) printf("%s/\n", cur->name); else printf("%s\n", cur->name);
        cur = cur->next;
    } while (cur != head);
}

static void build_abs_path(FileNode *dir, char *out, size_t outsz) 
{
    if (dir == sRootNode) 
    {
        snprintf(out, outsz, "/");
        return;
    }
    const char *stack[1024];
    int top = 0;
    FileNode *cur = dir;
    while (cur && cur != sRootNode && top < 1024) { stack[top++] = cur->name; cur = cur->parent; }
    size_t pos = 0;
    if (outsz>0) out[pos++]='/' ;
    for (int i=top-1;i>=0;--i) {
        size_t n = strlen(stack[i]);
        if (pos + n + 1 >= outsz) break;
        memcpy(out+pos, stack[i], n); pos+=n;
        if (i) out[pos++]='/';
    }
    if (pos<outsz) out[pos]='\0';
}

static int cmdCd(const char *name) {
    if (strcmp(name, ".") == 0) { }
            else if (strcmp(name, "..") == 0) { 
                if (sCmdNode->parent) sCmdNode = sCmdNode->parent; else sCmdNode = sRootNode; 
                char buf[256]; 
                build_abs_path(sCmdNode, buf, sizeof(buf)); 
                printf("Moved to %s\n", buf); }
            else if (strcmp(name, "/") == 0) { sCmdNode = sRootNode; printf("Moved to /\n"); }
            else {
                FileNode *n = findChild(sCmdNode, name);
                if (!n || !n->isDir) printf("No such directory\n");
                else { sCmdNode = n; char buf[256]; build_abs_path(sCmdNode, buf, sizeof(buf)); printf("Moved to %s\n", buf); }
            }
}

static void cmdPwd(void) 
{
    char buf[1024];
    build_abs_path(sCmdNode, buf, sizeof(buf));
    printf("%s\n", buf);
}

static void cmdDf(void) 
{
    int total = sNumberOfBlocks;
    int freeb = sFreeCount;
    int used = total - freeb;
    double usage = total ? (100.0*used/total) : 0.0;
    printf("Total Blocks: %d\nUsed Blocks: %d\nFree Blocks: %d\nDisk Usage: %.2f%%\n",
           total, used, freeb, usage);
}

static char* trim(char *s) 
{
    while (isspace((unsigned char)*s)) ++s;
    if (*s == 0) return s;
    char *e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e)) --e;
    e[1] = '\0';
    return s;
}

static const char* nextToken(char **p) 
{
    static char buf[4096];
    buf[0] = '\0';
    char *s = *p;
    while (*s && isspace((unsigned char)*s)) ++s;
    if (!*s) { *p = s; return buf; }
    if (*s == '"' || *s == '\'') {
        char quote = *s++;
        size_t k=0; 
        while (*s && *s != quote && k < sizeof(buf)-1) buf[k++] = *s++;
        buf[k] = '\0';
        if (*s == quote) ++s;
    } else {
        size_t k=0;
        while (*s && !isspace((unsigned char)*s) && k < sizeof(buf)-1) buf[k++] = *s++;
        buf[k]='\0';
    }
    while (*s && isspace((unsigned char)*s)) ++s;
    *p = s;
    return buf;
}

static void prompt(void) 
{
    char path[256];
    build_abs_path(sCmdNode, path, sizeof(path));
    printf("%s > ", path);
    fflush(stdout);
}

static void applycmd(void) {
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
    char line[8192];
    for (;;) {
        prompt();
        if (!fgets(line, sizeof(line), stdin)) break;
        char *s = trim(line);
        if (!*s) continue;

        char *cursor = s;
        const char *cmd = nextToken(&cursor);

        if (strcmp(cmd, "mkdir") == 0) {
            const char *name = nextToken(&cursor);
            if (!*name) { printf("Usage: mkdir <name>\n"); continue; }
            cmd_mkdir(name);
        } else if (strcmp(cmd, "create") == 0) {
            const char *name = nextToken(&cursor);
            if (!*name) { printf("Usage: create <name>\n"); continue; }
            cmd_create(name);
        } else if (strcmp(cmd, "write") == 0) 
        {
            const char *name = nextToken(&cursor);
            char *filename=malloc(strlen(name)+1);
            strcpy(filename,name);
            const char *payload = nextToken(&cursor);
            if (!*name) { printf("Usage: write <name> ""data""\n"); continue; }
            if (!*payload) payload = "";
            cmdWrite(filename, payload);
            free(filename);
        } else if (strcmp(cmd, "read") == 0) {
            const char *name = nextToken(&cursor);
            if (!*name) { printf("Usage: read <name>\n"); continue; }
            cmdRead(name);
        } else if (strcmp(cmd, "delete") == 0) {
            const char *name = nextToken(&cursor);
            if (!*name) { printf("Usage: delete <name>\n"); continue; }
            cmdDelete(name);
        } else if (strcmp(cmd, "rmdir") == 0) {
            const char *name = nextToken(&cursor);
            if (!*name) { printf("Usage: rmdir <name>\n"); continue; }
            cmdRmdir(name);
        } else if (strcmp(cmd, "ls") == 0) {
            cmdLs();
        } else if (strcmp(cmd, "cd") == 0) {
            const char *name = nextToken(&cursor);
            if (!*name) { printf("Usage: cd <dir>|..|/\n"); continue; }
            cmdCd(name);
        } else if (strcmp(cmd, "pwd") == 0) {
            cmdPwd();
        } else if (strcmp(cmd, "df") == 0) {
            cmdDf();
        } else if (strcmp(cmd, "exit") == 0) {
            printf("Memory released. Exiting program...\n");
            break;
        } else {
            printf("Unknown command: %s\n", cmd);
        }
    }
}
int main(void) 
{
    int blocks = DEFAULT_NUM_BLOCKS;
    initDiskAndVfs(blocks);
    applycmd();
    freeVFS();
    return 0;
}
