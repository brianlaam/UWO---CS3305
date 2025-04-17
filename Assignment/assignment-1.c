/*******************************************************************************
 * File: assignment-1.c
 * Compile: gcc -o assignment-1 assignment-1.c
 * Run Example: 
 *   time ./assignment-1 0 0 100   // Serial
 *   time ./assignment-1 1 0 100   // Parallel
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // for fork(), getpid(), getppid()
#include <sys/wait.h> // for wait()
#include <math.h>     // for sqrt()

/* 
   Helper function to check if a number n is prime.
   Returns 1 if prime, 0 if not prime.
*/
int isPrime(int n) {
    int i;
    if (n < 2) return 0;
    // Check divisors from 2 to sqrt(n)
    for (i = 2; i <= (int)sqrt((double)n); i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

/*
   Count and sum of primes in the interval [start, end].
   The results are returned via pointers: *prime_count, *prime_sum.
*/
void count_sum_primes(int start, int end, int *prime_count, long long *prime_sum) {
    int num;
    *prime_count = 0;
    *prime_sum = 0;

    for (num = start; num <= end; num++) {
        if (isPrime(num)) {
            (*prime_count)++;
            (*prime_sum) += num;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <serial(0)|parallel(!0)> <min> <max>\n", argv[0]);
        return 1;
    }

    // 1) Parse command-line arguments
    int mode         = atoi(argv[1]); // 0 = serial, !=0 = parallel
    int range_min    = atoi(argv[2]);
    int range_max    = atoi(argv[3]);

    // Print the parent's process ID as per assignment requirement
    pid_t parent_pid = getpid();
    printf("Process id: %d\n", parent_pid);

    // 2) Compute the size of each subrange
    int total_numbers = (range_max - range_min + 1);
    // Basic integer division for subrange
    int chunk_size    = total_numbers / 4;
    int remainder     = total_numbers % 4;

    // We will define each subrange [start_i, end_i].
    // The last subrange may contain the remainder if total_numbers is not multiple of 4.

    // subrange boundaries
    int start[4], end[4];
    int current_start = range_min;
    for (int i = 0; i < 4; i++) {
        start[i] = current_start;
        // Add chunk_size to cover a base slice
        int length = chunk_size;
        if (i == 3) {
            // Last one gets the remainder
            length += remainder;
        }
        end[i] = start[i] + length - 1;

        // Move to the next chunk
        current_start = end[i] + 1;
    }

    if (mode == 0) {
        /********************************************************
         * Serial Mode
         * The parent (this process) does all subranges in series.
         ********************************************************/
        for (int i = 0; i < 4; i++) {
            int pc = 0, long long psum = 0;
            count_sum_primes(start[i], end[i], &pc, &psum);

            // Print the results from the parent
            // (In serial mode, pid = ppid = parent_pid for all lines)
            printf("pid: %d, ppid %d - Count and sum of prime numbers between %d and %d are %d and %lld\n",
                   (int)getpid(), (int)getppid(), start[i], end[i], pc, psum);
        }
    } else {
        /********************************************************
         * Parallel Mode
         * We create 4 child processes. Each child handles one subrange.
         ********************************************************/
        pid_t pids[4];

        for (int i = 0; i < 4; i++) {
            pids[i] = fork();
            if (pids[i] < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            } 
            else if (pids[i] == 0) {
                // Child process
                int pc = 0, psum = 0;
                count_sum_primes(start[i], end[i], &pc, &psum);

                // Print the results from the child
                printf("pid: %d, ppid %d - Count and sum of prime numbers between %d and %d are %d and %lld\n",
                       (int)getpid(), (int)getppid(), start[i], end[i], pc, psum);

                // IMPORTANT: child should exit after printing
                exit(0);
            }
            // Parent continues the for-loop to spawn more children
        }

        // Parent must wait for all children to finish
        for (int i = 0; i < 4; i++) {
            wait(NULL);
        }
    }

    return 0;
}
