#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void printMatrix(int *matrix, int NumberOfRowsOrCols) 
{
    for (int i = 0; i < NumberOfRowsOrCols; i++) 
    {
        for (int j = 0; j < NumberOfRowsOrCols; j++)
            printf("%3d ", *(matrix + i * NumberOfRowsOrCols + j));
        printf("\n");
    }
    printf("\n");
}

void rotateMatrix(int *matrix, int NumberOfRowsOrCols) 
{
    int **matrixPtrs = malloc(NumberOfRowsOrCols * NumberOfRowsOrCols * sizeof(int *));
    for (int i = 0; i < NumberOfRowsOrCols * NumberOfRowsOrCols; i++) 
    {
        matrixPtrs[i] = matrix + i;
    }

    for (int layer = 0; layer < NumberOfRowsOrCols / 2; layer++) 
    {
        for (int i = layer; i < NumberOfRowsOrCols - layer - 1; i++) 
        {
            int **topPtr   = &matrixPtrs[layer * NumberOfRowsOrCols + i];
            int **rightPtr  = &matrixPtrs[i * NumberOfRowsOrCols + (NumberOfRowsOrCols - layer - 1)];
            int **bottomPtr = &matrixPtrs[(NumberOfRowsOrCols - layer - 1) * NumberOfRowsOrCols + (NumberOfRowsOrCols - i - 1)];
            int **leftPtr   = &matrixPtrs[(NumberOfRowsOrCols - i - 1) * NumberOfRowsOrCols + layer];

            int *tempPtr = *topPtr;
            *topPtr = *leftPtr;
            *leftPtr = *bottomPtr;
            *bottomPtr = *rightPtr;
            *rightPtr = tempPtr;
        }
    }

    int *tempMatrix = malloc(NumberOfRowsOrCols * NumberOfRowsOrCols * sizeof(int));
    for (int i = 0; i < NumberOfRowsOrCols * NumberOfRowsOrCols; i++) 
    {
        tempMatrix[i] = *(matrixPtrs[i]);
    }
    for (int i = 0; i < NumberOfRowsOrCols * NumberOfRowsOrCols; i++) 
    {
        matrix[i] = tempMatrix[i];
    }

    free(tempMatrix);
    free(matrixPtrs);
}

void smoothingFilter(int *matrix, int NumberOfRowsOrCols) 
{
    int *tempRow = malloc(NumberOfRowsOrCols * sizeof(int));

    for (int i = 0; i < NumberOfRowsOrCols; i++) 
    {
        for (int j = 0; j < NumberOfRowsOrCols; j++) 
        {
            int sum = 0, count = 0;
            for (int di = -1; di <= 1; di++) 
            {
                for (int dj = -1; dj <= 1; dj++) 
                {
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < NumberOfRowsOrCols && nj >= 0 && nj < NumberOfRowsOrCols) 
                    {
                        sum += *(matrix + ni * NumberOfRowsOrCols + nj);
                        count++;
                    }
                }
            }
            *(tempRow + j) = sum / count;
        }

        for (int j = 0; j < NumberOfRowsOrCols; j++) 
        {
            *(matrix + i * NumberOfRowsOrCols + j) = *(tempRow + j);
        }
    }

    free(tempRow);
}

int main() {
    srand(time(NULL));
    int inputMatrixSize;
    printf("Enter matrix size (2-10): ");
    scanf("%d", &inputMatrixSize);

    int *matrix = malloc(inputMatrixSize * inputMatrixSize * sizeof(int));

    for (int i = 0; i < inputMatrixSize; i++)
        for (int j = 0; j < inputMatrixSize; j++)
            *(matrix + i * inputMatrixSize + j) = rand() % 256;

    printf("\nOriginal Matrix:\n");
    printMatrix(matrix, inputMatrixSize);

    rotateMatrix(matrix, inputMatrixSize);
    printf("Matrix after 90° Clockwise Rotation:\n");
    printMatrix(matrix, inputMatrixSize);

    smoothingFilter(matrix, inputMatrixSize);
    printf("Matrix after 3x3 Smoothing Filter:\n");
    printMatrix(matrix, inputMatrixSize);

    free(matrix);
    return 0;
}
