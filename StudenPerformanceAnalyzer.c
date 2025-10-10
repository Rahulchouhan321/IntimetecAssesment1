#include <stdio.h>
#include <string.h>


struct Student
{
    int rollNo;
    char name[50];
    float marks[3];
};


int gTotalStudents = 0;


void inputStudent(struct Student *s);
float calculateTotal(struct Student s);
float calculateAverage(struct Student s);
char assignGrade(float avg);
void displayStudents(struct Student s[], int n);
void printStars(int count);
void printRollNumbersRecursively(struct Student s[], int index, int n);

int main() 
{
    struct Student students[50];
    int n;

    printf("=== Student Performance Analyzer ===\n\n");

    printf("Enter number of students: ");
    scanf("%d", &n);
    gTotalStudents = n;  


    
    for (int i = 0; i < n; i++) 
    {
        printf("\n--- Enter details for Student %d ---\n", i + 1);
        inputStudent(&students[i]);
    }

    
    displayStudents(students, n);

    
    printf("\nRoll Numbers (using recursion): ");
    printRollNumbersRecursively(students, 0, n);
    printf("\n");

    return 0;
}


void inputStudent(struct Student *s) 
{
    printf("Enter Roll Number: ");
    scanf("%d", &s->rollNo);
    getchar();

    printf("Enter Name: ");
    scanf(" %[^\n]", s->name);
    getchar();

    for (int i = 0; i < 3; i++) 
    {
        printf("Enter marks of subject %d: ", i + 1);
        scanf("%f", &s->marks[i]);
    }
}


float calculateTotal(struct Student s)
{
    float total = 0;  
    for (int i = 0; i < 3; i++) 
    {
        total += s.marks[i];
    }
    return total;
}


float calculateAverage(struct Student s)
{
    float total = calculateTotal(s); 
    return total / 3.0;
}


char assignGrade(float avg) 
{
    if (avg >= 85) return 'A';
    else if (avg >= 70) return 'B';
    else if (avg >= 50) return 'C';
    else if (avg >= 35) return 'D';
    else return 'F';
}


void printStars(int count) 
{
    for (int i = 0; i < count; i++) 
    {
        printf("* ");
    }
}


void displayStudents(struct Student s[], int n) 
{  
    printf("\n=== Student Performance Report ===\n");

    for (int i = 0; i < n; i++) 
    {
        float total = calculateTotal(s[i]);
        float average = calculateAverage(s[i]);
        char grade = assignGrade(average);

        printf("\nStudent %d:\n", i + 1);
        printf("Roll Number : %d\n", s[i].rollNo);
        printf("Name        : %s\n", s[i].name);
        printf("Total Marks : %.2f\n", total);
        printf("Average     : %.2f\n", average);
        printf("Grade       : %c\n", grade);

      
        if (grade == 'F') 
        {
            printf("Performance Pattern : Skipped (Failed)\n");
            continue;
        }

        int stars = 0;
        switch (grade) 
        {
            case 'A': stars = 5; break;
            case 'B': stars = 4; break;
            case 'C': stars = 3; break;
            case 'D': stars = 2; break;
        }

        printf("Performance Pattern : ");
        printStars(stars);
        printf("\n");
    }

    
}
void printRollNumbersRecursively(struct Student s[], int index, int n) 
{
    if (index == n) return;  
    printf("%d ", s[index].rollNo);
    printRollNumbersRecursively(s, index + 1, n); 
}
