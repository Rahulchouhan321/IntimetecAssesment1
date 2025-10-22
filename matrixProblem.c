#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_SONARIMAGE_SIZE 2
#define MAX_SONARIMAGE_SIZE 10
#define MIN_SONARIMAGE_VALUE 0
#define MAX_SONARIMAGE_VALUE 255

void printSonarImageMatrix(int *sonarImage, int sonarImageSize);
void initializeWithRandomValues(int *sonarImage, int sonarImageSize);
void rotateSonarImageClockwise(int *sonarImage, int sonarImageSize);
void transposeMatrix(int *sonarImage, int sonarImageSize);
void swapElements(int *firstElement, int *secondElement);
void applySmoothingFilter(int *sonarImage, int sonarImageSize);

int main()
{
    int sonarImageSize;
    printf("Enter matrix size (2-10): ");
    scanf("%d", &sonarImageSize);

    if (sonarImageSize < MIN_SONARIMAGE_SIZE || sonarImageSize > MAX_SONARIMAGE_SIZE)
    {
        printf("Invalid matrix size. Must be in range (2-10)");
        return 0;
    }

    int sonarImage[sonarImageSize][sonarImageSize];

    printf("Original Randomly Generated Matrix: \n");
    initializeWithRandomValues((int *)sonarImage, sonarImageSize);
    printSonarImageMatrix((int *)(sonarImage), sonarImageSize);

    printf("Matrix after 90 degree Clockwise Rotation: \n");
    rotateSonarImageClockwise((int *)sonarImage, sonarImageSize);
    printSonarImageMatrix((int *)(sonarImage), sonarImageSize);

    printf("Matrix after Applying 3X3 Smoothing Filter:  \n");
    applySmoothingFilter((int *)(sonarImage), sonarImageSize);
    printSonarImageMatrix((int *)(sonarImage), sonarImageSize);

    return 0;
}

void printSonarImageMatrix(int *sonarImage, int sonarImageSize)
{
    for (int rowIndex = 0; rowIndex < sonarImageSize; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < sonarImageSize; columnIndex++)
        {
            printf("%3d ", *(sonarImage + rowIndex * sonarImageSize + columnIndex));
        }

        printf("\n");
    }

    printf("\n");
}

void swapElements(int *firstElement, int *secondElement)
{
    int temporaryStore = *firstElement;
    *firstElement = *secondElement;
    *secondElement = temporaryStore;
}

void initializeWithRandomValues(int *sonarImage, int sonarImageSize)
{
    srand(time(0));

    for (int rowIndex = 0; rowIndex < sonarImageSize; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < sonarImageSize; columnIndex++)
        {
            *(sonarImage + rowIndex * sonarImageSize + columnIndex) = (rand() % (MAX_SONARIMAGE_VALUE - MIN_SONARIMAGE_VALUE + 1)) + MIN_SONARIMAGE_VALUE;
        }
    }
}

void transposeMatrix(int *sonarImage, int sonarImageSize)
{
    for (int rowIndex = 0; rowIndex < sonarImageSize; rowIndex++)
    {
        for (int columnIndex = rowIndex + 1; columnIndex < sonarImageSize; columnIndex++)
        {
            swapElements((sonarImage + rowIndex * sonarImageSize + columnIndex), (sonarImage + columnIndex * sonarImageSize + rowIndex));
        }
    }
}

void rotateSonarImageClockwise(int *sonarImage, int sonarImageSize)
{
    int *startColumn;
    int *endColumn;
    transposeMatrix((int *)(sonarImage), sonarImageSize);

    for (int rowIndex = 0; rowIndex < sonarImageSize; rowIndex++)
    {
        startColumn = sonarImage + rowIndex * sonarImageSize;
        endColumn = sonarImage + rowIndex * sonarImageSize + (sonarImageSize - 1);

        while (startColumn < endColumn)
        {
            swapElements(startColumn, endColumn);
            startColumn++;
            endColumn--;
        }
    }
}

void applySmoothingFilter(int *sonarImage, int sonarImageSize)
{
    int *temporaryRow = malloc(sonarImageSize * sizeof(int));

    for (int rowIndex = 0; rowIndex < sonarImageSize; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < sonarImageSize; columnIndex++)
        {
            int sumOfElements = 0;
            int totalElements = 0;

            
            for (int rowOffset = -1; rowOffset <= 1; rowOffset++)
            {
                for (int colOffset = -1; colOffset <= 1; colOffset++)
                {
                    int neighborRow = rowIndex + rowOffset;
                    int neighborCol = columnIndex + colOffset;

                    
                    if (neighborRow >= 0 && neighborRow < sonarImageSize &&
                        neighborCol >= 0 && neighborCol < sonarImageSize)
                    {
                        sumOfElements += *(sonarImage + neighborRow * sonarImageSize + neighborCol);
                        totalElements++;
                    }
                }
            }

            
            *(temporaryRow + columnIndex) = sumOfElements / totalElements;
        }

       
        for (int columnIndex = 0; columnIndex < sonarImageSize; columnIndex++)
        {
            *(sonarImage + rowIndex * sonarImageSize + columnIndex) = *(temporaryRow + columnIndex);
        }
    }

    free(temporaryRow);
}
