#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "users.txt"

struct User {
    int id;
    char name[100];
    int age;
};


void createUser() {
    struct User u;
    FILE *fp = fopen(FILENAME, "a");
    if (!fp) {
        printf("Error opening file!\n");
        return;
    }

    printf("Enter ID: ");
    scanf("%d", &u.id);
    //getchar(); 

    printf("Enter Name: ");
    scanf("%[^\n]", u.name); 
    //getchar();

    printf("Enter Age: ");
    scanf("%d", &u.age);

    fprintf(fp, "%d|%s|%d\n", u.id, u.name, u.age); 
    fclose(fp);
    printf("User added successfully!\n");
}

void readUsers() {
    struct User u;
    FILE *fp = fopen(FILENAME, "r");
    char line[200];

    if (!fp) {
        printf("No users found (file does not exist).\n");
        return;
    }

    printf("\n--- User Records ---\n");
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d|%[^|]|%d", &u.id, u.name, &u.age) == 3) {
            printf("ID: %d | Name: %s | Age: %d\n", u.id, u.name, u.age);
        }
    }
    fclose(fp);
}

// Function to update user by ID
void updateUser() {
    int id, found = 0;
    struct User u;
    FILE *fp = fopen(FILENAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    char line[200];

    if (!fp) {
        printf("No users to update.\n");
        return;
    }

    printf("Enter ID of user to update: ");
    scanf("%d", &id);
    getchar();

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d|%[^|]|%d", &u.id, u.name, &u.age) == 3) {
            if (u.id == id) {
                found = 1;
                printf("Enter new Name: ");
                scanf("%[^\n]", u.name);
                getchar();
                printf("Enter new Age: ");
                scanf("%d", &u.age);
            }
            fprintf(temp, "%d|%s|%d\n", u.id, u.name, u.age);
        }
    }

    fclose(fp);
    fclose(temp);

    remove(FILENAME);
    rename("temp.txt", FILENAME);

    if (found)
        printf("User updated successfully!\n");
    else
        printf("User with ID %d not found.\n", id);
}


void deleteUser() {
    int id, found = 0;
    struct User u;
    FILE *fp = fopen(FILENAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    char line[200];

    if (!fp) {
        printf("No users to delete.\n");
        return;
    }

    printf("Enter ID of user to delete: ");
    scanf("%d", &id);

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d|%[^|]|%d", &u.id, u.name, &u.age) == 3) {
            if (u.id == id) {
                found = 1; // skip this record
            } else {
                fprintf(temp, "%d|%s|%d\n", u.id, u.name, u.age);
            }
        }
    }

    fclose(fp);
    fclose(temp);

    remove(FILENAME);
    rename("temp.txt", FILENAME);

    if (found)
        printf("User deleted successfully!\n");
    else
        printf("User with ID %d not found.\n", id);
}

int main() {
    int choice;

    do {
        printf("\n--- User Management ---\n");
        printf("1. Create User\n");
        printf("2. Read Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: createUser(); break;
            case 2: readUsers(); break;
            case 3: updateUser(); break;
            case 4: deleteUser(); break;
            case 5: printf("Exiting...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 5);

    return 0;
}
