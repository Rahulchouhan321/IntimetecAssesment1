#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "players_data.h"

#define MAX_NAME_LEN 50
#define ROLE_ALLROUND 3
#define ROLE_BATSMAN 1
#define ROLE_BOWLER 2

typedef struct PlayerNode {
    int PlayerId;
    char Name[MAX_NAME_LEN];
    char TeamName[MAX_NAME_LEN];
    int Role;
    int TotalRuns;
    float BattingAverage;
    float StrikeRate;
    int Wickets;
    float EconomyRate;
    double PerformanceIndex;
    struct PlayerNode* next;
} PlayerNode;

typedef struct TeamNode {
    int TeamId;
    char Name[MAX_NAME_LEN];
    PlayerNode* head;
    int totalPlayers;
    double avgBattingStrikeRate;

    PlayerNode** batsmen;
    int batsmen_count;
    int batsmen_capacity;

    PlayerNode** bowlers;
    int bowlers_count;
    int bowlers_capacity;

    PlayerNode** allrounders;
    int allrounders_count;
    int allrounders_capacity;
} TeamNode;
typedef struct HeapNode {
    int t;  
    int pos;
    double perf;
} HeapNode;

typedef struct {
    HeapNode* data;
    int size;
    int cap;
} SimpleHeap;

static TeamNode teamsGlobal[20];

int getInt() {
    int x;
    char ch;

    while (1) 
    {
        if (scanf("%d", &x) == 1) 
        {
            while ((ch = getchar()) != '\n' && ch != EOF);

            return x; 
        }
        printf("Invalid input! Try again.\n");   
        while ((ch = getchar()) != '\n' && ch != EOF);
    }
}
float getFloat() {
    float x;
    char ch;
    while (1) 
    {
        if (scanf("%f", &x) == 1) 
        {
            while ((ch = getchar()) != '\n' && ch != EOF);
            return x;
        }
        printf("Invalid input! Try again.\n");
        while ((ch = getchar()) != '\n' && ch != EOF);
    }
}

int roleStringToInt(const char* r) {
    if (strcmp(r, "Batsman") == 0) return ROLE_BATSMAN;
    if (strcmp(r, "Bowler") == 0) return ROLE_BOWLER;
    if (strcmp(r, "All-rounder") == 0) return ROLE_ALLROUND;
    return 0;
}

double computePerformanceIndexForNode(const PlayerNode* PlayerPtr) {
    if (PlayerPtr->Role == ROLE_BATSMAN)
        return (PlayerPtr->BattingAverage * PlayerPtr->StrikeRate) / 100.0;

    if (PlayerPtr->Role == ROLE_BOWLER)
        return (PlayerPtr->Wickets * 2.0) + (100.0 - PlayerPtr->EconomyRate);

    return (PlayerPtr->BattingAverage * PlayerPtr->StrikeRate) / 100.0 + (PlayerPtr->Wickets * 2.0);
}

PlayerNode* createNodeFromHeaderPlayer(const Player* headPtr) {
    PlayerNode* n = malloc(sizeof(PlayerNode));
    if (!n) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);   
    }
    strcpy(n->Name, headPtr->name);
    strcpy(n->TeamName, headPtr->team);
    n->PlayerId = headPtr->id;
    n->Role = roleStringToInt(headPtr->role);
    n->TotalRuns = headPtr->totalRuns;
    n->BattingAverage = headPtr->battingAverage;
    n->StrikeRate = headPtr->strikeRate;
    n->Wickets = headPtr->wickets;
    n->EconomyRate = headPtr->economyRate;
    n->PerformanceIndex = computePerformanceIndexForNode(n);
    n->next = NULL;
    return n;
}

PlayerNode* createNodeFromInput(int id, const char* name, const char* team, int role,
                                int runs, float avg, float sr, int wk, float er) {
    PlayerNode* n = malloc(sizeof(PlayerNode));
    if (!n) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);   
    }
    strcpy(n->Name, name);
    strcpy(n->TeamName, team);
    n->PlayerId = id;
    n->Role = role;
    n->TotalRuns = runs;
    n->BattingAverage = avg;
    n->StrikeRate = sr;
    n->Wickets = wk;
    n->EconomyRate = er;
    n->PerformanceIndex = computePerformanceIndexForNode(n);
    n->next = NULL;
    return n;
}

void ensureCapacity(PlayerNode*** playerArray, int* cap, int needed) {
    if (*cap >= needed) return;
    int newcap = (*cap == 0) ? 8 : (*cap * 2);
    while (newcap < needed) newcap *= 2;

    PlayerNode** newarr = malloc(newcap * sizeof(PlayerNode*));
    if (!newarr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);   
    }
    if (*playerArray) memcpy(newarr, *playerArray, sizeof(PlayerNode*) * (*cap));
    free(*playerArray);
    *playerArray = newarr;
    *cap = newcap;
    
}

void insertIntoSortedArray(PlayerNode*** playerArray, int* size, int* cap, PlayerNode* n) {
    ensureCapacity(playerArray, cap, (*size) + 1);
    int i = (*size) - 1;

    while (i >= 0 && (*playerArray)[i]->PerformanceIndex < n->PerformanceIndex) {
        (*playerArray)[i+1] = (*playerArray)[i];
        i--;
    }
    (*playerArray)[i+1] = n;
    (*size)++;
}

void initializeTeamsFromHeader() {
    for (int i = 0; i < teamCount; ++i) {
        teamsGlobal[i].TeamId = i + 1;
        strcpy(teamsGlobal[i].Name, teams[i]);
        teamsGlobal[i].head = NULL;
        teamsGlobal[i].totalPlayers = 0;
        teamsGlobal[i].avgBattingStrikeRate = 0;

        teamsGlobal[i].batsmen = NULL;
        teamsGlobal[i].bowlers = NULL;
        teamsGlobal[i].allrounders = NULL;

        teamsGlobal[i].batsmen_count = 0;
        teamsGlobal[i].bowlers_count = 0;
        teamsGlobal[i].allrounders_count = 0;

        teamsGlobal[i].batsmen_capacity = 0;
        teamsGlobal[i].bowlers_capacity = 0;
        teamsGlobal[i].allrounders_capacity = 0;
    }
}

int findTeamIndexById(int teamID) {
    int l = 0, r = teamCount - 1;
    while (l <= r) {
        int mid = (l+r)/2;
        if (teamsGlobal[mid].TeamId == teamID) return mid;
        if (teamsGlobal[mid].TeamId < teamID) l = mid + 1;
        else r = mid - 1;
    }
    return -1;
}

void recomputeTeamAvgSR(TeamNode* teamNode) {
    double sum = 0;
    int count = 0;
    PlayerNode* p = teamNode->head;
    while (p) {
        if (p->Role == ROLE_BATSMAN || p->Role == ROLE_ALLROUND) {
            sum += p->StrikeRate;
            count++;
        }
        p = p->next;
    }
    teamNode->avgBattingStrikeRate = (count == 0 ? 0 : sum / count);

    teamNode->totalPlayers = 0;
    p = teamNode->head;
    while (p) { teamNode->totalPlayers++; p = p->next; }
}

void loadPlayersFromHeader() {
    for (int i = 0; i < playerCount; i++) {
        PlayerNode* PlayerPtr = createNodeFromHeaderPlayer(&players[i]);

        int idx = -1;
        for (int t = 0; t < teamCount; t++) {
            if (strcmp(teamsGlobal[t].Name, PlayerPtr->TeamName) == 0) {
                idx = t; break;
            }
        }
        if (idx == -1) { free(PlayerPtr); continue; }

        TeamNode* T = &teamsGlobal[idx];

        if (!T->head) T->head = PlayerPtr;
        else {
            PlayerNode* c = T->head;
            while (c->next) c = c->next;
            c->next = PlayerPtr;
        }

        if (PlayerPtr->Role == ROLE_BATSMAN)
            insertIntoSortedArray(&T->batsmen, &T->batsmen_count, &T->batsmen_capacity, PlayerPtr);
        else if (PlayerPtr->Role == ROLE_BOWLER)
            insertIntoSortedArray(&T->bowlers, &T->bowlers_count, &T->bowlers_capacity, PlayerPtr);
        else
            insertIntoSortedArray(&T->allrounders, &T->allrounders_count, &T->allrounders_capacity, PlayerPtr);
    }

    for (int i = 0; i < teamCount; i++)
        recomputeTeamAvgSR(&teamsGlobal[i]);
}

void printPlayerHeader(int showTeam) {
    if (showTeam) {
        printf("======================================================================================\n");
        printf("ID   Name                          Team             Role         Runs  Avg   SR   Wkts  ER   PerfIndex\n");
        printf("======================================================================================\n");
    } else {
        printf("====================================================================================\n");
        printf("ID   Name                        Role        Runs  Avg   SR   Wkts  ER   PerfIndex\n");
        printf("====================================================================================\n");
    }
}

void printPlayerNode(PlayerNode* playerPtr, int showTeam) {
    if (showTeam) {
        printf("%-4d %-28s %-15s %-12s %5d %5.1f %5.1f %5d %5.1f %9.2f\n",
            playerPtr->PlayerId, playerPtr->Name, playerPtr->TeamName,
            (playerPtr->Role==1?"Batsman":playerPtr->Role==2?"Bowler":"All-rounder"),
            playerPtr->TotalRuns, playerPtr->BattingAverage, playerPtr->StrikeRate,
            playerPtr->Wickets, playerPtr->EconomyRate, playerPtr->PerformanceIndex
        );
    } else {
        printf("%-4d %-25s %-12s %5d %5.1f %5.1f %5d %5.1f %9.2f\n",
            playerPtr->PlayerId, playerPtr->Name,
            (playerPtr->Role==1?"Batsman":playerPtr->Role==2?"Bowler":"All-rounder"),
            playerPtr->TotalRuns, playerPtr->BattingAverage, playerPtr->StrikeRate,
            playerPtr->Wickets, playerPtr->EconomyRate, playerPtr->PerformanceIndex
        );
    }
}

void addPlayerToTeamInteractive() {
    int teamId;
    printf("Enter Team ID: ");
    teamId=getInt();

    int idx = findTeamIndexById(teamId);
    if (idx == -1) { printf("Invalid team\n"); return; }

    TeamNode* teamNode = &teamsGlobal[idx];

    int pid, role, runs, wk;
    float avg, sr, er;
    char name[64];

    printf("Player ID: ");
    pid=getInt();
    getchar();
    printf("Name: ");
    fgets(name, 64, stdin);
    name[strcspn(name, "\n")] = 0;

    printf("Role (1=Batsman, 2=Bowler, 3=All-rounder): ");
    role=getInt();

    printf("Total Runs: "); 
    runs=getInt();
    printf("Batting Average: "); 
    avg=getFloat();
    printf("Strike Rate: "); 
    sr=getFloat();
    printf("Wickets: ");
    wk=getInt();
    printf("Economy Rate: "); 
    er=getFloat();

    PlayerNode* playerNode = createNodeFromInput(pid, name, teamNode->Name, role, runs, avg, sr, wk, er);

    if (!teamNode->head) teamNode->head = playerNode;
    else {
        PlayerNode* c = teamNode->head;
        while (c->next) c = c->next;
        c->next = playerNode;
    }

    if (role == ROLE_BATSMAN)
        insertIntoSortedArray(&teamNode->batsmen, &teamNode->batsmen_count, &teamNode->batsmen_capacity, playerNode);
    else if (role == ROLE_BOWLER)
        insertIntoSortedArray(&teamNode->bowlers, &teamNode->bowlers_count, &teamNode->bowlers_capacity, playerNode);
    else
        insertIntoSortedArray(&teamNode->allrounders, &teamNode->allrounders_count, &teamNode->allrounders_capacity, playerNode);
    recomputeTeamAvgSR(teamNode);
    printf("Player added successfully!\n");
}

void displayPlayersOfTeam() {
    int teamId;
    printf("Enter Team ID: ");
    teamId=getInt();

    int idx = findTeamIndexById(teamId);
    if (idx == -1) { printf("Invalid Team\n"); return; }

    TeamNode* teamNode = &teamsGlobal[idx];
    printPlayerHeader(0);

    PlayerNode* p = teamNode->head;
    while (p) {
        printPlayerNode(p, 0);
        p = p->next;
    }

    printf("====================================================================================\n");
    printf("Total Players: %d\n", teamNode->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", teamNode->avgBattingStrikeRate);
}

int cmpTeamAvgSR(const void* a, const void* b) {
    const TeamNode* A = a;
    const TeamNode* B = b;
    if (A->avgBattingStrikeRate < B->avgBattingStrikeRate) return 1;
    if (A->avgBattingStrikeRate > B->avgBattingStrikeRate) return -1;
    return 0;
}

void displayTeamsByAvgSR() {
    TeamNode temp[64];
    memcpy(temp, teamsGlobal, sizeof(TeamNode)*teamCount);

    qsort(temp, teamCount, sizeof(TeamNode), cmpTeamAvgSR);

    printf("=========================================================\n");
    printf("ID  Team Name                AvgSR    Total\n");
    printf("=========================================================\n");

    for (int i = 0; i < teamCount; i++) {
        printf("%-3d %-23s %7.2f  %5d\n",
            temp[i].TeamId, temp[i].Name,
            temp[i].avgBattingStrikeRate,
            temp[i].totalPlayers
        );
    }
    printf("=========================================================\n");
}

void displayTopKOfTeamByRole() {
    int teamId, role, K;
    printf("Team ID: ");
    teamId=getInt();
    printf("Role (1/2/3): "); 
    role=getInt();
    printf("K: ");
    K=getInt();

    int idx = findTeamIndexById(teamId);
    if (idx == -1) { printf("Invalid Team\n"); return; }

    TeamNode* T = &teamsGlobal[idx];
    PlayerNode** arr = NULL;
    int count = 0;

    if (role == ROLE_BATSMAN) { arr = T->batsmen; count = T->batsmen_count; }
    else if (role == ROLE_BOWLER) { arr = T->bowlers; count = T->bowlers_count; }
    else { arr = T->allrounders; count = T->allrounders_count; }

    printPlayerHeader(0);

    for (int i = 0; i < count && i < K; i++)
        printPlayerNode(arr[i], 0);
}


void heapInit(SimpleHeap* h) {
    h->cap = teamCount + 5;
    h->data = malloc(sizeof(HeapNode)*h->cap);
    if (!h->data) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);   

    }
    h->size = 0;
}

void swapHN(HeapNode* a, HeapNode* b) {
    HeapNode t = *a; *a = *b; *b = t;
}

void heapPush(SimpleHeap* h, HeapNode x) {
    if (h->size == h->cap) {
        h->cap *= 2;
        h->data = realloc(h->data, sizeof(HeapNode)*h->cap);
        if (!h->data) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);   
    }
    }
    h->data[h->size] = x;
    int i = h->size++;
    while (i > 0) {
        int parent = (i-1)/2;
        if (h->data[parent].perf >= h->data[i].perf) break;
        swapHN(&h->data[parent], &h->data[i]);
        i = parent;
    }
}

int heapEmpty(SimpleHeap* h) { return h->size == 0; }

HeapNode heapPop(SimpleHeap* h) {
    HeapNode ret = h->data[0];
    h->size--;
    if (h->size > 0) {
        h->data[0] = h->data[h->size];
        int i = 0;
        while (1) {
            int l = 2*i+1, r = 2*i+2, largest = i;
            if (l < h->size && h->data[l].perf > h->data[largest].perf) largest = l;
            if (r < h->size && h->data[r].perf > h->data[largest].perf) largest = r;
            if (largest == i) break;
            swapHN(&h->data[i], &h->data[largest]);
            i = largest;
        }
    }
    return ret;
}

void heapFree(SimpleHeap* h) {
    free(h->data);
}

void displayAllTeamsByRole() {
    int role;
    printf("Role (1/2/3): ");
    role=getInt();

    SimpleHeap h;
    heapInit(&h);

    for (int t = 0; t < teamCount; t++) {
        TeamNode* T = &teamsGlobal[t];

        if (role == ROLE_BATSMAN && T->batsmen_count > 0)
            heapPush(&h, (HeapNode){t, 0, T->batsmen[0]->PerformanceIndex});
        else if (role == ROLE_BOWLER && T->bowlers_count > 0)
            heapPush(&h, (HeapNode){t, 0, T->bowlers[0]->PerformanceIndex});
        else if (role == ROLE_ALLROUND && T->allrounders_count > 0)
            heapPush(&h, (HeapNode){t, 0, T->allrounders[0]->PerformanceIndex});
    }

    printPlayerHeader(1);

    while (!heapEmpty(&h)) {
        HeapNode top = heapPop(&h);
        int t = top.t;
        int pos = top.pos;

        PlayerNode* p = NULL;

        if (role == ROLE_BATSMAN) p = teamsGlobal[t].batsmen[pos];
        else if (role == ROLE_BOWLER) p = teamsGlobal[t].bowlers[pos];
        else p = teamsGlobal[t].allrounders[pos];

        printPlayerNode(p, 1);

        int next = pos + 1;

        if (role == ROLE_BATSMAN && next < teamsGlobal[t].batsmen_count)
            heapPush(&h, (HeapNode){t, next, teamsGlobal[t].batsmen[next]->PerformanceIndex});

        else if (role == ROLE_BOWLER && next < teamsGlobal[t].bowlers_count)
            heapPush(&h, (HeapNode){t, next, teamsGlobal[t].bowlers[next]->PerformanceIndex});

        else if (role == ROLE_ALLROUND && next < teamsGlobal[t].allrounders_count)
            heapPush(&h, (HeapNode){t, next, teamsGlobal[t].allrounders[next]->PerformanceIndex});
    }

    heapFree(&h);
}

void freePlayerList(PlayerNode* head) {
    PlayerNode* c = head;
    while (c) {
        PlayerNode* next = c->next;
        free(c);
        c = next;
    }
}

void freeTeam(TeamNode* T) {
    freePlayerList(T->head);
    free(T->batsmen);
    free(T->bowlers);
    free(T->allrounders);
}

void freeAllTeams() {
    for (int i = 0; i < teamCount; i++)
        freeTeam(&teamsGlobal[i]);
}

void printMenu() {
    printf("=============================================================================\n");
    printf("ICC ODI Player Performance Analyzer\n");
    printf("=============================================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display all Players of specific role Across All Teams\n");
    printf("6. Exit\n");
    printf("=============================================================================\n");
    printf("Enter your choice: ");
}

int main() {
    initializeTeamsFromHeader();
    loadPlayersFromHeader();

    while (1) {
        printMenu();
        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1: addPlayerToTeamInteractive(); break;
            case 2: displayPlayersOfTeam(); break;
            case 3: displayTeamsByAvgSR(); break;
            case 4: displayTopKOfTeamByRole(); break;
            case 5: displayAllTeamsByRole(); break;
            case 6:
                printf("Exiting...\n");
                freeAllTeams();
                return 0;
            default: printf("Invalid choice\n");
        }

        printf("\n");
    }
}
