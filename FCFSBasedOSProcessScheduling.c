#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    

#define HASH_SIZE 101
#define MAX_KILLS 64

typedef enum {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED,
    KILLED
} State;

typedef struct ProcessControlBlock {
    int processId;
    char name[32];

    int burstTime;
    int ioStart;
    int ioDuration;

    int cpuUsed;
    int ioUsed;
    int execSinceStart;
    int remainingIo;

    int arrivalTime;
    int completionTime;

    State state;
    struct ProcessControlBlock *next;
} ProcessControlBlock;

typedef struct Queue {
    ProcessControlBlock *front;
    ProcessControlBlock *rear;
} Queue;

typedef struct HashNode {
    int processId;
    ProcessControlBlock *pcb;
    struct HashNode *next;
} HashNode;

typedef struct KillEvent {
    int processId;
    int time;
} KillEvent;

HashNode *gHashTable[HASH_SIZE];
KillEvent gKillEvents[MAX_KILLS];
int gKillEventCount = 0;

Queue gReadyQueue;
Queue gWaitingQueue;
Queue gTerminatedQueue;

int gCurrentTime = 0;
int gTotalProcesses = 0;
int gTerminatedProcesses = 0;

ProcessControlBlock *gRunningProcess = NULL;

int gTickDurationSeconds = 0;

void initQueue(Queue *queue) {
    queue->front = queue->rear = NULL;
}

void enqueueProcess(Queue *queue, ProcessControlBlock *pcb) {
    pcb->next = NULL;
    if (queue->rear == NULL) {
        queue->front = queue->rear = pcb;
    } else {
        queue->rear->next = pcb;
        queue->rear = pcb;
    }
}

ProcessControlBlock* dequeueProcess(Queue *queue) {
    if (queue->front == NULL) return NULL;
    ProcessControlBlock *pcb = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL) queue->rear = NULL;
    pcb->next = NULL;
    return pcb;
}

void removeFromQueue(Queue *queue, ProcessControlBlock *pcb) {
    ProcessControlBlock *current = queue->front;
    ProcessControlBlock *previous = NULL;

    while (current) {
        if (current == pcb) {
            if (previous == NULL) {
                queue->front = current->next;
                if (queue->front == NULL) queue->rear = NULL;
            } else {
                previous->next = current->next;
                if (current == queue->rear) queue->rear = previous;
            }
            current->next = NULL;
            return;
        }
        previous = current;
        current = current->next;
    }
}

int isQueueEmpty(Queue *queue) {
    return queue->front == NULL;
}

unsigned int hashPid(int processId) {
    return (unsigned int)processId % HASH_SIZE;
}

void putProcessInMap(ProcessControlBlock *pcb) {
    unsigned int index = hashPid(pcb->processId);
    HashNode *node = (HashNode *)malloc(sizeof(HashNode));
    node->processId = pcb->processId;
    node->pcb = pcb;
    node->next = gHashTable[index];
    gHashTable[index] = node;
}

ProcessControlBlock* getProcessFromMap(int processId) {
    unsigned int index = hashPid(processId);
    HashNode *current = gHashTable[index];
    while (current) {
        if (current->processId == processId) return current->pcb;
        current = current->next;
    }
    return NULL;
}

void addKillEvent(int processId, int time) {
    gKillEvents[gKillEventCount].processId = processId;
    gKillEvents[gKillEventCount].time = time;
    gKillEventCount++;
}

void applyKillEventsForTime(int time) {
    for (int i = 0; i < gKillEventCount; i++) {
        if (gKillEvents[i].time == time) {
            int processId = gKillEvents[i].processId;
            ProcessControlBlock *pcb = getProcessFromMap(processId);
            if (!pcb || pcb->state == TERMINATED || pcb->state == KILLED) {
                continue;
            }

            if (gRunningProcess == pcb) {
                gRunningProcess = NULL;
            }

            if (pcb->state == READY) {
                removeFromQueue(&gReadyQueue, pcb);
            } else if (pcb->state == WAITING) {
                removeFromQueue(&gWaitingQueue, pcb);
            }

            pcb->state = KILLED;
            pcb->completionTime = time;
            enqueueProcess(&gTerminatedQueue, pcb);
            gTerminatedProcesses++;

            printf("[time %d] KILL event applied on PID %d\n", time, processId);
        }
    }
}

void updateWaitingQueue() {
    ProcessControlBlock *current = gWaitingQueue.front;
    ProcessControlBlock *previous = NULL;

    while (current) {
        ProcessControlBlock *next = current->next;

        current->remainingIo--;
        current->ioUsed++;

        if (current->remainingIo <= 0) {
            current->state = READY;
            current->execSinceStart = 0;

            if (previous == NULL) {
                gWaitingQueue.front = next;
            } else {
                previous->next = next;
            }
            if (current == gWaitingQueue.rear) {
                gWaitingQueue.rear = previous;
            }

            current->next = NULL;
            enqueueProcess(&gReadyQueue, current);

            printf("[time %d] IO complete → PID %d moved to READY\n",
                   gCurrentTime, current->processId);
        } else {
            previous = current;
        }

        current = next;
    }
}

void simulateTick() {
    sleep(1);

    updateWaitingQueue();

    if (gRunningProcess) {
        gRunningProcess->cpuUsed++;
        gRunningProcess->execSinceStart++;

        printf("[time %d] Running PID %d (cpuUsed=%d)\n",
               gCurrentTime, gRunningProcess->processId, gRunningProcess->cpuUsed);

        if (gRunningProcess->ioDuration > 0 &&
            gRunningProcess->execSinceStart == gRunningProcess->ioStart && gRunningProcess->ioUsed ==0) {

            gRunningProcess->remainingIo = gRunningProcess->ioDuration;
            gRunningProcess->state = WAITING;
            enqueueProcess(&gWaitingQueue, gRunningProcess);
            printf("[time %d] PID %d moved to WAITING (IO start)\n",
                   gCurrentTime + 1, gRunningProcess->processId);

            gRunningProcess = NULL;
        }
        else if (gRunningProcess->cpuUsed >= gRunningProcess->burstTime) {
            gRunningProcess->state = TERMINATED;
            gRunningProcess->completionTime = gCurrentTime + 1;
            enqueueProcess(&gTerminatedQueue, gRunningProcess);

            gTerminatedProcesses++;
            printf("[time %d] PID %d TERMINATED\n",
                   gCurrentTime + 1, gRunningProcess->processId);

            gRunningProcess = NULL;
        }
    }
}

void freePcb(ProcessControlBlock *pcb) {
    if (pcb) free(pcb);
}

void freeQueue(Queue *queue) {
    ProcessControlBlock *current = queue->front;
    while (current) {
        ProcessControlBlock *next = current->next;
        freePcb(current);
        current = next;
    }
    queue->front = queue->rear = NULL;
}

void freeHashMap() {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode *current = gHashTable[i];
        while (current) {
            HashNode *next = current->next;
            free(current);
            current = next;
        }
        gHashTable[i] = NULL;
    }
}

void printFinalResults() {
    printf("\n========= FINAL RESULTS =========\n");
    printf("PID\tName\tCPU\tIO\tTurnaround\tWaiting\n");

    ProcessControlBlock *current = gTerminatedQueue.front;

    while (current) {
        int turnaround = current->completionTime - current->arrivalTime;
        int waiting = turnaround - current->burstTime;
        if (waiting < 0) waiting = 0;

        printf("%d\t%s\t%d\t%d\t%d\t\t%d",
               current->processId,
               current->name,
               current->cpuUsed,
               current->ioUsed,
               turnaround,
               waiting);

        if (current->state == KILLED) {
            printf("\t(KILLED)");
        }
        printf("\n");

        current = current->next;
    }
}
int readProcessInput(ProcessControlBlock *pcb) {
    char line[256];

    while (1) {
        printf("Enter: <name> <processId> <burst> <io_start> <io_duration>\n> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("Invalid input. Try again.\n");
            continue;
        }

        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) {
            printf("Invalid input. Try again.\n");
            continue;
        }

        char name[32], ioStartStr[16], ioDurationStr[16];
        int pid, burst;

        int fields = sscanf(line, "%31s %d %d %15s %15s",
                            name, &pid, &burst, ioStartStr, ioDurationStr);

        if (fields < 5) {
            printf("Invalid format. Try again.\n");
            continue;
        }

        if (pid <= 0 || burst <= 0 || strlen(name) == 0) {
            printf("Invalid values. Try again.\n");
            continue;
        }

        int ioStart, ioDuration;

        if (strcmp(ioStartStr, "-") == 0) ioStart = -1;
        else ioStart = atoi(ioStartStr);

        if (ioStart < -1) {
            printf("Invalid IO start. Try again.\n");
            continue;
        }

        if (strcmp(ioDurationStr, "-") == 0) ioDuration = 0;
        else ioDuration = atoi(ioDurationStr);

        if (ioDuration < 0) {
            printf("Invalid IO duration. Try again.\n");
            continue;
        }

        strcpy(pcb->name, name);
        pcb->processId = pid;
        pcb->burstTime = burst;
        pcb->ioStart = ioStart;
        pcb->ioDuration = ioDuration;

        return 1;
    }
}
int readKillEvent(int *processId, int *time) {
    char line[128];

    while (1) {
        printf("Enter KILL <processId> <time>:\n> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("Invalid input. Try again.\n");
            continue;
        }

        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) {
            printf("Invalid input. Try again.\n");
            continue;
        }

        char cmd[16];
        int pid, t;

        int fields = sscanf(line, "%15s %d %d", cmd, &pid, &t);

        if (fields != 3) {
            printf("Invalid format. Try again.\n");
            continue;
        }

        if (strcasecmp(cmd, "KILL") != 0) {
            printf("Input must start with KILL. Try again.\n");
            continue;
        }

        if (pid <= 0 || t < 0) {
            printf("Values must be positive. Try again.\n");
            continue;
        }

        *processId = pid;
        *time = t;
        return 1;
    }
}



int main(void) {
    initQueue(&gReadyQueue);
    initQueue(&gWaitingQueue);
    initQueue(&gTerminatedQueue);

    printf("Enter number of processes: ");
    scanf("%d", &gTotalProcesses);

    for (int i = 0; i < gTotalProcesses; i++) {
        ProcessControlBlock *pcb = (ProcessControlBlock *)malloc(sizeof(ProcessControlBlock));
        memset(pcb, 0, sizeof(ProcessControlBlock));

         readProcessInput(pcb);
    

        pcb->arrivalTime = 0;
        pcb->state = READY;

        putProcessInMap(pcb);
        enqueueProcess(&gReadyQueue, pcb);
    }

    int killCountInput;
    printf("Enter number of KILL events: ");
    scanf("%d", &killCountInput);

    for (int i = 0; i < killCountInput; i++) {
        int processId, time;
        readKillEvent(&processId, &time);
        addKillEvent(processId, time);
    }

    while (gTerminatedProcesses < gTotalProcesses) {
        applyKillEventsForTime(gCurrentTime);

        if (!gRunningProcess) {
            gRunningProcess = dequeueProcess(&gReadyQueue);
            if (gRunningProcess) {
                gRunningProcess->state = RUNNING;
                printf("[time %d] PID %d scheduled to RUN\n",
                       gCurrentTime, gRunningProcess->processId);
            }
        }

        if (!gRunningProcess && isQueueEmpty(&gWaitingQueue)) {
            
            gCurrentTime++;
            continue;
        }

        simulateTick();
        gCurrentTime++;
    }

    printFinalResults();

    freeQueue(&gTerminatedQueue);
    freeQueue(&gReadyQueue);
    freeQueue(&gWaitingQueue);
    freeHashMap();

    printf("\nMemory cleanup completed.\n");
    return 0;
}
