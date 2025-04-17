#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9
#define THREADS 27

int sudoku[SIZE][SIZE];

int valid[THREADS] = {0};

typedef struct {
    int row;
    int col;
    int index;
    int type;
    int thread_num;
} parameters;

// Function to check a 3×3 subgrid.
// A subgrid index i is mapped as: row = (i/3)*3, col = (i%3)*3.
void *check_subgrid(void *param) {
    parameters *p = (parameters *) param;
    int startRow = p->row;
    int startCol = p->col;
    int seen[10] = {0}; 

    for (int i = startRow; i < startRow + 3; i++) {
        for (int j = startCol; j < startCol + 3; j++) {
            int num = sudoku[i][j];
            if (seen[num] == 1) {
                valid[p->thread_num - 1] = 0;
                printf("Thread # %2d (subgrid %d) is INVALID\n", p->thread_num, p->index + 1);
                pthread_exit(0);
            }
            seen[num] = 1;
        }
    }
    valid[p->thread_num - 1] = 1;
    printf("Thread # %2d (subgrid %d) is valid\n", p->thread_num, p->index + 1);
    pthread_exit(0);
}

// Function to check a row
void *check_row(void *param) {
    parameters *p = (parameters *) param;
    int row = p->index;
    int seen[10] = {0};
    
    for (int j = 0; j < SIZE; j++) {
        int num = sudoku[row][j];
        if (seen[num] == 1) {
            valid[p->thread_num - 1] = 0;
            printf("Thread # %2d (row %d) is INVALID\n", p->thread_num, row + 1);
            pthread_exit(0);
        }
        seen[num] = 1;
    }
    valid[p->thread_num - 1] = 1;
    printf("Thread # %2d (row %d) is valid\n", p->thread_num, row + 1);
    pthread_exit(0);
}

// Function to check a column
void *check_column(void *param) {
    parameters *p = (parameters *) param;
    int col = p->index;
    int seen[10] = {0};
    
    for (int i = 0; i < SIZE; i++) {
        int num = sudoku[i][col];
        if (seen[num] == 1) {
            valid[p->thread_num - 1] = 0;
            printf("Thread # %2d (column %d) is INVALID\n", p->thread_num, col + 1);
            pthread_exit(0);
        }
        seen[num] = 1;
    }
    valid[p->thread_num - 1] = 1;
    printf("Thread # %2d (column %d) is valid\n", p->thread_num, col + 1);
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    // Read the 9×9 grid from the file
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fscanf(fp, "%d", &sudoku[i][j]);
        }
    }
    fclose(fp);

    pthread_t threads[THREADS];
    parameters params[THREADS];
    int threadIndex = 0; 

    // Create threads for checking 3×3 subgrids, 9 in total
    // Compute starting row and column for each subgrid i
    for (int i = 0; i < SIZE; i++) {
        params[threadIndex].type = 0;
        params[threadIndex].index = i;
        params[threadIndex].row = (i / 3) * 3;
        params[threadIndex].col = (i % 3) * 3;
        params[threadIndex].thread_num = threadIndex + 1;
        pthread_create(&threads[threadIndex], NULL, check_subgrid, (void *)&params[threadIndex]);
        threadIndex++;
    }

    // Create threads for checking rows, 9 in total
    for (int i = 0; i < SIZE; i++) {
        params[threadIndex].type = 1;
        params[threadIndex].index = i;
        params[threadIndex].thread_num = threadIndex + 1;
        pthread_create(&threads[threadIndex], NULL, check_row, (void *)&params[threadIndex]);
        threadIndex++;
    }

    //  Create threads for checking columns,  9 in total
    for (int i = 0; i < SIZE; i++) {
        params[threadIndex].type = 2;
        params[threadIndex].index = i;
        params[threadIndex].thread_num = threadIndex + 1;
        pthread_create(&threads[threadIndex], NULL, check_column, (void *)&params[threadIndex]);
        threadIndex++;
    }

    // Wait for all 27 threads to complete their execution.
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Return the overall results
    int validPuzzle = 1;
    for (int i = 0; i < THREADS; i++) {
        if (valid[i] == 0) {
            validPuzzle = 0;
            break;
        }
    }

    if (validPuzzle)
        printf("%s contains a valid solution\n", argv[1]);
    else
        printf("%s contains an INVALID solution.\n", argv[1]);

    return 0;
}
