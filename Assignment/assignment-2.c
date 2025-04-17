#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MIN_NUM 1000
#define MAX_NUM 9999

void writeInt(int fd, int value) {
    if (write(fd, &value, sizeof(int)) != sizeof(int)) {
        perror("Error occurs during: Write");
        exit(EXIT_FAILURE);
    }
}

void readInt(int fd, int *value) {
    ssize_t n = read(fd, value, sizeof(int));
    if (n < 0) {
        perror("Error occurs during: Read");
        exit(EXIT_FAILURE);
    } else if (n == 0) {
        fprintf(stderr, "Pipe closed unexpectedly.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    // Arguments Checking
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <4-digit number> <4-digit number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Convert arguments to integers
    int num1 = atoi(argv[1]);
    int num2 = atoi(argv[2]);

    // Intergers' range checking
    if (num1 < MIN_NUM || num1 > MAX_NUM || num2 < MIN_NUM || num2 > MAX_NUM) {
        fprintf(stderr, "Both input numbers should be within the range of: %d to %d.\n", MIN_NUM, MAX_NUM);
        exit(EXIT_FAILURE);
    }

    // Output Message
    printf("Your integers are %d %d\n", num1, num2);

    int a1 = num1 / 100;
    int a2 = num1 % 100;
    int b1 = num2 / 100;
    int b2 = num2 % 100;

    int pipe_p2c[2]; // Pipe from parent to child
    int pipe_c2p[2]; // Pipe from child to parent

    if (pipe(pipe_p2c) == -1) {
        perror("Error occurs in Pipe: Parent to Child");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_c2p) == -1) {
        perror("Error occurs in Pipe: Child to Parent");
        exit(EXIT_FAILURE);
    }

    // Fork the process to create a child.
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error occurs during: Fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child Process Code
        close(pipe_p2c[1]); // Close the write end and the child reads
        close(pipe_c2p[0]); // Close the read end and the child writes

        int x, y;  // To store the numbers read from pipe
        
        // A = a1 * b1
        readInt(pipe_p2c[0], &x);
        printf("Child (PID %d): Received %d from parent\n", getpid(), x);
        readInt(pipe_p2c[0], &y);
        printf("Child (PID %d): Received %d from parent\n", getpid(), y);
        int product = x * y;
        printf("Child (PID %d): Sending %d to parent\n", getpid(), product);
        writeInt(pipe_c2p[1], product);

        // C = a1 * b2
        readInt(pipe_p2c[0], &x);
        printf("Child (PID %d): Received %d from parent\n", getpid(), x);
        readInt(pipe_p2c[0], &y);
        printf("Child (PID %d): Received %d from parent\n", getpid(), y);
        product = x * y;
        printf("Child (PID %d): Sending %d to parent\n", getpid(), product);
        writeInt(pipe_c2p[1], product);

        // B = a2 * b1
        readInt(pipe_p2c[0], &x);
        printf("Child (PID %d): Received %d from parent\n", getpid(), x);
        readInt(pipe_p2c[0], &y);
        printf("Child (PID %d): Received %d from parent\n", getpid(), y);
        product = x * y;
        printf("Child (PID %d): Sending %d to parent\n", getpid(), product);
        writeInt(pipe_c2p[1], product);

        // D = a2 * b2
        readInt(pipe_p2c[0], &x);
        printf("Child (PID %d): Received %d from parent\n", getpid(), x);
        readInt(pipe_p2c[0], &y);
        printf("Child (PID %d): Received %d from parent\n", getpid(), y);
        product = x * y;
        printf("Child (PID %d): Sending %d to parent\n", getpid(), product);
        writeInt(pipe_c2p[1], product);

        // Close the ends and exit.
        close(pipe_p2c[0]);
        close(pipe_c2p[1]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent Process Code
        close(pipe_p2c[0]); // Close the write end and the parent writes
        close(pipe_c2p[1]); // Close the write end and the parent reads

        printf("Parent (PID %d): created child (PID %d)\n", getpid(), pid);

        int result, A, B, C, D;
        long X, Y, Z, final_result;
        int totalDigits = 4;
        int halfDigits = 2;

        // Calculate X = A * 10^4, 
        // where A = a1 * b1
        printf("\n### \n# Calculating X \n###\n");

        // Send a1 and b1 to child.
        printf("Parent (PID %d): Sending %d to child\n", getpid(), a1);
        writeInt(pipe_p2c[1], a1);
        printf("Parent (PID %d): Sending %d to child\n", getpid(), b1);
        writeInt(pipe_p2c[1], b1);

        // Receive product A from child.
        readInt(pipe_c2p[0], &result);
        A = result;
        printf("Parent (PID %d): Received %d from child\n", getpid(), A);
        // X = A * 10^(totalDigits) = A * 10^4
        X = A;
        for (int i = 0; i < totalDigits; i++) {
            X *= 10;
        }

        // Calculate Y = (B + C) * 10^(halfDigits), 
        // where C = a1 * b2 and B = a2 * b1.
        printf("\n### \n# Calculating Y \n###\n");

        // Send a1 and b2 to child to calculate C.
        printf("Parent (PID %d): Sending %d to child\n", getpid(), a1);
        writeInt(pipe_p2c[1], a1);
        printf("Parent (PID %d): Sending %d to child\n", getpid(), b2);
        writeInt(pipe_p2c[1], b2);
        readInt(pipe_c2p[0], &result);
        C = result;
        printf("Parent (PID %d): Received %d from child\n", getpid(), C);

        // Send a2 and b1 to child to calculate B.
        printf("Parent (PID %d): Sending %d to child\n", getpid(), a2);
        writeInt(pipe_p2c[1], a2);
        printf("Parent (PID %d): Sending %d to child\n", getpid(), b1);
        writeInt(pipe_p2c[1], b1);
        readInt(pipe_c2p[0], &result);
        B = result;
        printf("Parent (PID %d): Received %d from child\n", getpid(), B);

        // Calculate Y = (B + C) * 10^(halfDigits) = (B+C)*10^2
        Y = (B + C);
        for (int i = 0; i < halfDigits; i++) {
            Y *= 10;
        }

        // Calculate Z = D * 10^0 = D, 
        // where D = a2 * b2
        printf("\n### \n# Calculating Z \n###\n");

        // Send a2 and b2 to child.
        printf("Parent (PID %d): Sending %d to child\n", getpid(), a2);
        writeInt(pipe_p2c[1], a2);
        printf("Parent (PID %d): Sending %d to child\n", getpid(), b2);
        writeInt(pipe_p2c[1], b2);
        readInt(pipe_c2p[0], &result);
        D = result;
        printf("Parent (PID %d): Received %d from child\n", getpid(), D);

        // Z = D * 10^0 = D.
        Z = D;
        final_result = X + Y + Z;
        printf("\n%d*%d == %ld + %ld + %ld == %ld\n", num1, num2, X, Y, Z, final_result);

        // Close the ends and exit.
        close(pipe_p2c[1]);
        close(pipe_c2p[0]);
        wait(NULL);
    }
    return 0;
}
