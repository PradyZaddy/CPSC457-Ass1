// CPSC 457 Assignment 1 Part B
// Authors: Nathaniel Appiah, Pradhyuman Nandal

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <math.h>

// Provided prime number searcher from assignment page
int is_prime(int num) {
    if (num < 2) return 0;
    for (int i = 2; i <= sqrt(num); i++) {
    if (num % i == 0) return 0;
    }
    return 1;
}

int main (int argc, char *args[]) {
    // If input arguments are incorrect upon starting the program.
    // Checks if there is 4 arguments in total (program, lower bound, upper bound, number of processes)
    if (argc != 4) {
        fprintf(stderr, "Input error. Program input must be integers in the following format: \n [Lower Bound] [Upper Bound] [Number of processes]");  // Error message for incorrect input
        printf("\n");   // New line purely for formatting purposes
        exit(1);    // Generic Error program exit
    }

    int LOWER_BOUND = atoi(args[1]);    // Upper Bound Argument
    int UPPER_BOUND = atoi(args[2]);    // Lower Bound Argument
    int N = atoi(args[3]);              // Number of Processes Argument.

    // Setting error for invalid lower bound
    if (LOWER_BOUND > UPPER_BOUND) {
        fprintf(stderr, "Lower bound must be less than upper bound");
        printf("\n");   // New line purely for formatting purposes

        exit(1);
    }

    // Setting error for invalid upper bound
    if (UPPER_BOUND < LOWER_BOUND) {
        fprintf(stderr, "Upper bound must be greater than lower bound");
        printf("\n");   // New line purely for formatting purposes
        exit(1);
    }

    // Setting error for invalid number of processes
    if (N <= 0) {
        fprintf(stderr, "Number of processes (N) must be greater than 0");
        printf("\n");   // New line purely for formatting purposes
        exit(1);
    }

    int number_of_elements_between_bounds = (UPPER_BOUND - LOWER_BOUND + 1);     // Count of the amount of elements between the bounds (inclusive)

    // Conditional ensures number of processes (N) never exceeds number of elements between bounds
    if (N > number_of_elements_between_bounds) {
        fprintf(stderr, "Number of processes (N) cannot be greater than the number of elements between bounds");
        printf("\n");   // New line only for formatting purposes
        exit(1);
    }



    // According to the advice of the genius TA Samuel, I have made this variable embody the size of the child blocks to be the amount of elements between the bounds
    // Replaces my former Dummy value of 1000 integers maximum to adjust size of each block according to the input.
    // +1 to account for prime number count
    size_t block_ints_in_bytes_efficient = (1 + number_of_elements_between_bounds) * sizeof(int);
    
    // Total memory allocated for all processes (N) altogether.
    // Named "SIZE" as per assignment document highlights
    size_t SIZE = block_ints_in_bytes_efficient * N;

    // 4000 bytes allocated to ensure a maximum of 1000 integers + plus the 4 bytes for the count of the amount of primes, since each integer is 4 bytes
    int shmid = shmget(IPC_PRIVATE, SIZE, IPC_CREAT | 0666);

    // Access shared memory to read and write through mem
    int *shm_ptr = (int *) shmat(shmid, NULL, 0);

    // Now each child must only write a set bound of prime numbers to ensure zero overlap
    // First determine size of each child bound to allocate each child's personal block
    int child_bound_size = (UPPER_BOUND - LOWER_BOUND + 1) / N;

    // If the range is not evenly divisible by the number of processes, take remainder and keep track of it.
    int remainder = (UPPER_BOUND - LOWER_BOUND + 1) % N;
    
    // Initial fork for generating child processes
    // For loop to run each child process one at a time
    for (int i = 0; i < N; i++) {
        // Initialize pid to prep forking
        pid_t pid;
        pid = fork(); // Fork a new child
    
        // process id = 0  resembles a child process
        if (pid == 0) {
            // Determing the child bound for the particular child to restrict access to itself to prevent all overlapping
            int current_child_lower_bound = LOWER_BOUND + (child_bound_size * i);               // Lower Bound of block
            int current_child_upper_bound = LOWER_BOUND + ((child_bound_size * (i + 1)) - 1);   // Upper Bound of block

            // Conditional restricted to just the final child to have how ever many extra elements based on what remainder is.
            if (i == N-1) {

                // Simply extends the upper bound by however manyy ticks
                current_child_upper_bound += remainder;
            }

            // To write the total count of primes each child has found, child_block divides each child by their allocated block in shm_ptr
            // We allocate each child to its own block by marking the beginning of its section in the shared memory array.
            // With this, the child process can access and append the prime number data into its own block
            int *child_block = shm_ptr + i * block_ints_in_bytes_efficient / sizeof(int);

            // initialize the count of primes found in range, starting from 0.
            int prime_number_count = 0;

            // Print the child pid with getpid() along with the current range that is being checked.
            fprintf(stdout, "Child PID %d checking range [%d, %d]\n", getpid(), current_child_lower_bound, current_child_upper_bound);
            
            // Loop through all numbers in the bounds of the current child and determine if each number is prime or not
            for (int number = current_child_lower_bound; number <= current_child_upper_bound; number++) {
                if (is_prime(number)) {
                    // Add the found prime number to the block, in a line starting from the immediate right side
                    // of the prime number count, and extending outwards for each new prime number
                    child_block[1 + prime_number_count] = number;

                    // Increase the count of prime numbers found by one
                    prime_number_count ++;
                }
            }

            // At the end, take the newly computed total amoiunt of prime numbers found into the 0th index of the child block
            child_block[0] = prime_number_count;

            // This child has successfully completed their process and can now terminate through _exit(0)
            _exit(0);
        } else if (pid < 0) {       // If the process ID is less than 0, this is neither a child or parent process (zombie) and straight up error
            // Print into the error console that the fork has failed
            fprintf(stderr, "Fork Failed!");

            // terminate the program through a generic error program exit
            exit(1);
        } 
    }

    for (int j = 0; j < N; j++) {
        // Uses wait(NULL) to have the parent process wait for all of the N children to complete their tasks and terminate
        wait(NULL);
    }

    // After parent has waited for all children to successfully finish, output that all the children have finished, and begin formatting for output.
    fprintf(stdout, "Parent: All children finished. Primes found:\n");

    // Initializes a child block for each child once again and access their locations in the block to
    // print out each child's found prime numbers in their respective allocated range
    for (int k = 0; k < N; k++) {
        // Access the beginning of each childs block
        int *child_block = shm_ptr + k * (block_ints_in_bytes_efficient / sizeof(int));

        // Initialize the prime number count to access the 0th index of each child's block to display the found value in the output
        int prime_number_count = child_block[0];

        // For loop will cycle all the way through each child's block and print out the prime numbers the child found in their allocated range,
        // and combines all of the children's found prime numbers altogether into a long list to display.
        for (int l = 0; l < prime_number_count; l++) {
            fprintf(stdout, "%d ", child_block[1 + l]); // Prints each prime number one by one
        }

    }       
    // new line for formatting purposes
    printf("\n");

    // Detaches the shared memory and removes it after its use.
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);
}

    