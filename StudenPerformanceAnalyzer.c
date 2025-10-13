#include <stdio.h>
#include <string.h>


struct Student
{
    int rollNo;
    char name[50];
    float marks[3];
};


int gTotalStudents = 0;

char assignGrade(float avg);
void displayStudents(struct Student s[], int n);
float calculateAverage(struct Student s);
float calculateTotal(struct Student s);
int getValidRollNo();
float getValidMarks(int subjectNo);
void inputStudent(struct Student *s);
void printRollNumbersRecursively(struct Student s[], int index, int n);
void printStars(int count);


int main() 
{
    struct Student students[50];
    int numberOfStudents;;

    printf("=== Student Performance Analyzer ===\n\n");

    printf("Enter number of students: ");
    scanf("%d", &numberOfStudents);
    gTotalStudents = numberOfStudents;  


    for (int i = 0; i < numberOfStudents; i++) 
    {
        printf("\n--- Enter details for Student %d ---\n", i + 1);
        inputStudent(&students[i]);
    }

    
    displayStudents(students, numberOfStudents);

    
    printf("\nRoll Numbers (using recursion): ");
    printRollNumbersRecursively(students, 0, numberOfStudents);
    printf("\n");

    return 0;
}


void inputStudent(struct Student *s) 
{
    s->rollNo = getValidRollNo();

    
    do 
    {
        printf("Enter Name: ");
        scanf(" %[^\n]", s->name);
        if (strlen(s->name) == 0)
        {
            printf("Name cannot be empty! Please enter again.\n");
        }
        if (strlen(s->name) >=50)
        {
            printf("Name can be smaller than 50 characters! Please enter again.\n");
        }

    } while (strlen(s->name) == 0);

    
    for (int i = 0; i < 3; i++) 
    {
        s->marks[i] = getValidMarks(i + 1);
    }
}


int getValidRollNo() 
{
    int rollNo;
    do 
    {
        printf("Enter Roll Number (positive integer): ");
        scanf("%d", &rollNo);
        if (rollNo <= 0) 
        {
            printf("Invalid Roll Number! Please enter again.\n");
        }
    } while (rollNo <= 0);
    return rollNo;
}


float getValidMarks(int subjectNo) 
{
    float marks;
    do {
        printf("Enter marks of subject %d (0-100): ", subjectNo);
        scanf("%f", &marks);
        if (marks < 0 || marks > 100) 
        {
            printf("Invalid Marks! Please enter between 0 and 100.\n");
        }
    } while (marks < 0 || marks > 100);
    return marks;
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
