#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NUM_ACCOUNTS 4 
#define NUM_ATMS 4 
#define INITIAL_CAPACITY 100 

typedef struct {
    int atm;
    int account;
    int amount;
} Transaction;

// Account balances.
int accountBalances[NUM_ACCOUNTS];

// One mutex per account
pthread_mutex_t account_mutex[NUM_ACCOUNTS];

// Transactions
Transaction *transactions = NULL;
int numTransactions = 0;
int transactionCapacity = 0;

// readTransactionsFromFile function:
// Open file -> Read header -> loads the starting balance
void readTransactionsFromFile(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    char line[256];
    
    // Read and ignore header line (e.g., "ATM, Account, Amount")
    if (!fgets(line, sizeof(line), fp)) {
        fprintf(stderr, "Error reading header\n");
        exit(EXIT_FAILURE);
    }
    
    // Read the next 4 lines for starting balances
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        if (!fgets(line, sizeof(line), fp)) {
            fprintf(stderr, "Error reading starting balance for account %d\n", i + 1);
            exit(EXIT_FAILURE);
        }
        char *token = strtok(line, ",");
        token = strtok(NULL, ","); 
        int account = atoi(token);
        token = strtok(NULL, ",");
        int balance = atoi(token);
        
        // Save the starting balance
        accountBalances[account - 1] = balance;
    }
    
    // Array for transactions
    transactionCapacity = INITIAL_CAPACITY;
    transactions = malloc(transactionCapacity * sizeof(Transaction));
    if (transactions == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    // Read remaining transactions lines
    while (fgets(line, sizeof(line), fp)) {
        if (strlen(line) < 3) continue;
        
        Transaction t;
        char *token = strtok(line, ",");
        if (!token) continue;
        t.atm = atoi(token); 
        
        token = strtok(NULL, ",");
        if (!token) continue;
        t.account = atoi(token); 
        
        token = strtok(NULL, ",");
        if (!token) continue;
        t.amount = atoi(token);
        
        if (numTransactions >= transactionCapacity) {
            transactionCapacity *= 2;
            transactions = realloc(transactions, transactionCapacity * sizeof(Transaction));
            if (!transactions) {
                fprintf(stderr, "Memory reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        transactions[numTransactions++] = t;
    }
    fclose(fp);
}

// atmThread function: 
// Scans through the transactions array
void *atmThread(void *arg) {
    int atmNumber = *(int *)arg;
    free(arg); 
    
    for (int i = 0; i < numTransactions; i++) {
        // Process only transactions for this ATM thread.
        if (transactions[i].atm == atmNumber) {
            int accIndex = transactions[i].account - 1; 
            
            pthread_mutex_lock(&account_mutex[accIndex]);
            
            // Process deposit or withdrawal.
            if (transactions[i].amount >= 0) {
                // Deposit
                accountBalances[accIndex] += transactions[i].amount;
                printf("Thread %d: Deposit $%d into Account %d\n", 
                       atmNumber, transactions[i].amount, transactions[i].account);
            } else {
                // Withdrawal
                int withdrawal = -transactions[i].amount;
                if (accountBalances[accIndex] >= withdrawal) {
                    accountBalances[accIndex] -= withdrawal;
                    printf("Thread %d: Withdraw $%d from Account %d\n", 
                           atmNumber, withdrawal, transactions[i].account);
                } else {
                    printf("Thread %d: Withdraw $%d from Account %d - INSUFFICIENT FUNDS\n", 
                           atmNumber, withdrawal, transactions[i].account);
                }
            }
            // Exit critical section: unlock the mutex.
            pthread_mutex_unlock(&account_mutex[accIndex]);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <transactions_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // Initialize the mutexes for each account.
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_init(&account_mutex[i], NULL);
    }
    
    // Read the transactions: balances and transactions
    readTransactionsFromFile(argv[1]);
    
    // Print the starting balances.
    printf("Starting Balances\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: $%d\n", i + 1, accountBalances[i]);
    }
    
    // Create one thread per ATM.
    pthread_t threads[NUM_ATMS];
    for (int i = 0; i < NUM_ATMS; i++) {
        // Allocate memory to pass the ATM number to the thread.
        int *atmNumber = malloc(sizeof(int));
        if (atmNumber == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
        *atmNumber = i + 1;
        int rc = pthread_create(&threads[i], NULL, atmThread, atmNumber);
        if (rc) {
            fprintf(stderr, "Error: pthread_create failed for ATM %d\n", i + 1);
            exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 0; i < NUM_ATMS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Print the final balances
    printf("\nFinal Balances\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: $%d\n", i + 1, accountBalances[i]);
    }
    
    // Destroy the mutexes & Free the transactions array.
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&account_mutex[i]);
    }
    free(transactions);
    
    return 0;
}
