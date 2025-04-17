#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Structure to hold process information */
typedef struct {
    int id;             // process id (P0, P1, etc.)
    int arrival;        // arrival time (for this assignment: equals process id)
    int burst;          // original burst time (from input)
    int remaining;      // remaining burst time
    int wait;           // total wait time
    int turnaround;     // total turnaround time
    bool finished;      // flag indicating whether process is finished
} Process;

/* Function prototypes */
int readInputFile(const char *filename, Process **processes);
void simulateFCFS(Process *processes, int n);
void simulateSJF(Process *processes, int n);
void simulateRR(Process *processes, int n, int timeQuantum);
void printProcessSummary(Process *processes, int n);

// Function to Read CSV file
int readInputFile(const char *filename, Process **processes) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char line[256];

    // Count lines with valid numeric burst data
    while (fgets(line, sizeof(line), fp)) {
        // Remove trailing newline.
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0)
            continue;

        char *token = strtok(line, ",");
        if (!token)
            continue;

        // Trim leading whitespace.
        while (*token && isspace(*token))
            token++;

        int burst = 0;
        if (isalpha(*token)) {
            token = strtok(NULL, ",");
            if (!token)
                continue;
            while (*token && isspace(*token))
                token++;
            if (!isdigit(*token) && *token != '-')
                continue; 
            burst = atoi(token);
        } else if (isdigit(*token) || *token == '-') {
            burst = atoi(token);
        } else {
            continue;
        }
        count++;
    }

    if (count == 0) {
        fclose(fp);
        return 0;
    }

    // Allocate the process array.
    *processes = (Process *)malloc(count * sizeof(Process));
    if (*processes == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    //  Read and store the burst times
    rewind(fp);
    int index = 0;
    while (fgets(line, sizeof(line), fp) && index < count) {
        // Remove trailing newline.
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0)
            continue;

        char *token = strtok(line, ",");
        if (!token)
            continue;
        while (*token && isspace(*token))
            token++;

        int burst = 0;
        if (isalpha(*token)) {
            token = strtok(NULL, ",");
            if (!token)
                continue;
            while (*token && isspace(*token))
                token++;
            if (!isdigit(*token) && *token != '-')
                continue;
            burst = atoi(token);
        } else if (isdigit(*token) || *token == '-') {
            burst = atoi(token);
        } else {
            continue;
        }
        (*processes)[index].id = index;
        (*processes)[index].arrival = index;
        (*processes)[index].burst = burst;
        (*processes)[index].remaining = burst;
        (*processes)[index].wait = 0;
        (*processes)[index].turnaround = 0;
        (*processes)[index].finished = false;
        index++;
    }

    fclose(fp);
    return count;
}

// FCFS Algorithm 
void simulateFCFS(Process *processes, int n) {
    int time = 0;
    int current = 0;
    printf("First Come First Served\n");
    
    while (true) {
        bool allFinished = true;
        for (int i = 0; i < n; i++) {
            if (!processes[i].finished) {
                allFinished = false;
                break;
            }
        }
        if (allFinished)
            break;

        if (current < n && processes[current].finished) {
            current++;
            continue;
        }

        int active = -1;
        if (current < n && processes[current].arrival <= time && !processes[current].finished) {
            active = current;
        }

        if (active != -1) {
            printf("T%d : P%d - Burst left %d, Wait time %d, Turnaround time %d\n",
                   time, processes[active].id, processes[active].remaining,
                   processes[active].wait, processes[active].turnaround);
        } else {
            printf("T%d : Idle\n", time);
        }

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival <= time && !processes[i].finished) {
                processes[i].turnaround++;
                if (i != active)
                    processes[i].wait++;
            }
        }

        if (active != -1) {
            processes[active].remaining--;
            if (processes[active].remaining <= 0) {
                processes[active].finished = true;
            }
        }
        time++;
    }
    printProcessSummary(processes, n);
}

 // SJF Algorithm 
void simulateSJF(Process *processes, int n) {
    int time = 0;
    int current = -1;
    printf("Shortest Job First\n");

    while (true) {
        bool allFinished = true;
        for (int i = 0; i < n; i++) {
            if (!processes[i].finished) {
                allFinished = false;
                break;
            }
        }
        if (allFinished)
            break;

        int selected = -1;
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival <= time && !processes[i].finished) {
                if (selected == -1)
                    selected = i;
                else if (processes[i].remaining < processes[selected].remaining)
                    selected = i;
                else if (processes[i].remaining == processes[selected].remaining) {
                    if (current == selected) {
                    } else if (current == i) {
                        selected = i;
                    }
                }
            }
        }
        current = selected;

        if (current != -1) {
            printf("T%d : P%d - Burst left %d, Wait time %d, Turnaround time %d\n",
                   time, processes[current].id, processes[current].remaining,
                   processes[current].wait, processes[current].turnaround);
        } else {
            printf("T%d : Idle\n", time);
        }

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival <= time && !processes[i].finished) {
                processes[i].turnaround++;
                if (i != current)
                    processes[i].wait++;
            }
        }

        if (current != -1) {
            processes[current].remaining--;
            if (processes[current].remaining <= 0) {
                processes[current].finished = true;
            }
        }
        time++;
    }
    printProcessSummary(processes, n);
}

// RR Algorithm
void simulateRR(Process *processes, int n, int timeQuantum) {
    int time = 0;
    int current = -1; 
    int quantumCounter = 0;

    int *queue = (int *)malloc(n * sizeof(int));
    int front = 0, rear = 0, count = 0;
    bool *inQueue = (bool *)calloc(n, sizeof(bool));

    printf("Round Robin with Quantum %d\n", timeQuantum);

    while (true) {
        bool allFinished = true;
        for (int i = 0; i < n; i++) {
            if (!processes[i].finished) {
                allFinished = false;
                break;
            }
        }
        if (allFinished)
            break;

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival == time && !processes[i].finished && !inQueue[i] && i != current) {
                queue[rear] = i;
                rear = (rear + 1) % n;
                count++;
                inQueue[i] = true;
            }
        }

        if (current == -1 && count > 0) {
            current = queue[front];
            front = (front + 1) % n;
            count--;
            inQueue[current] = false;
            quantumCounter = 0;
        }

        if (current != -1) {
            printf("T%d : P%d - Burst left %d, Wait time %d, Turnaround time %d\n",
                   time, processes[current].id, processes[current].remaining,
                   processes[current].wait, processes[current].turnaround);
        } else {
            printf("T%d : Idle\n", time);
        }

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival <= time && !processes[i].finished) {
                processes[i].turnaround++;
                if (i != current)
                    processes[i].wait++;
            }
        }

        if (current != -1) {
            processes[current].remaining--;
            quantumCounter++;
            if (processes[current].remaining <= 0) {
                processes[current].finished = true;
                current = -1;
                quantumCounter = 0;
            } else if (quantumCounter == timeQuantum) {
                queue[rear] = current;
                rear = (rear + 1) % n;
                count++;
                current = -1;
                quantumCounter = 0;
            }
        }
        time++;
    }

    free(queue);
    free(inQueue);
    printProcessSummary(processes, n);
}

// Print Result
void printProcessSummary(Process *processes, int n) {
    double totalWait = 0, totalTurnaround = 0;
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\n", processes[i].id);
        printf("\tWaiting time: %8d\n", processes[i].wait);
        printf("\tTurnaround time: %8d\n", processes[i].turnaround);
        totalWait += processes[i].wait;
        totalTurnaround += processes[i].turnaround;
    }
    printf("\nTotal average waiting time:     %.1f\n", totalWait / n);
    printf("Total average turnaround time:  %.1f\n", totalTurnaround / n);
}

int main(int argc, char *argv[]) {
    char algorithm;
    int timeQuantum = 0; 
    char *filename;

    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  ./assignment-4 -f inputFile\n");
        fprintf(stderr, "  ./assignment-4 -s inputFile\n");
        fprintf(stderr, "  ./assignment-4 -r quantum inputFile\n");
        exit(EXIT_FAILURE);
    }

    if (argv[1][0] != '-' || (argv[1][1] != 'f' && argv[1][1] != 's' && argv[1][1] != 'r')) {
        fprintf(stderr, "Invalid scheduling option. Use -f, -s, or -r.\n");
        exit(EXIT_FAILURE);
    }
    algorithm = argv[1][1];

    if (algorithm == 'r') {
        if (argc != 4) {
            fprintf(stderr, "Round Robin requires a quantum and an input file.\n");
            exit(EXIT_FAILURE);
        }
        timeQuantum = atoi(argv[2]);
        if (timeQuantum <= 0) {
            fprintf(stderr, "Time quantum must be a positive integer.\n");
            exit(EXIT_FAILURE);
        }
        filename = argv[3];
    } else {
        if (argc != 3) {
            fprintf(stderr, "Usage: ./assignment-4 -f inputFile   OR   ./assignment-4 -s inputFile\n");
            exit(EXIT_FAILURE);
        }
        filename = argv[2];
    }

    Process *processes = NULL;
    int numProcesses = readInputFile(filename, &processes);
    if (numProcesses <= 0) {
        fprintf(stderr, "No valid process data read from file.\n");
        exit(EXIT_FAILURE);
    }

    if (algorithm == 'f') {
        simulateFCFS(processes, numProcesses);
    } else if (algorithm == 's') {
        simulateSJF(processes, numProcesses);
    } else if (algorithm == 'r') {
        simulateRR(processes, numProcesses, timeQuantum);
    }

    free(processes);
    return 0;
}
