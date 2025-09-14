#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main (int argc, char *args[])
{

    int row = 100;
    int column = 1000;
    int treasureMatrix[row][column];

    for (int i = 0; i < 100; i++)
    {
        int rc = fork();

        if (rc == 0) // CHILD 
        {
            for (int j = 0; j < 1000; j++)
            {
                printf("Child %d (PID: %d): Searching row %d", i, getpid(), i);

                if (treasureMatrix[i][j] == 1)
                {
                    exit(j); // return the column where the treasure is found
                }

                // exit(i); // returning the row number that signifies the process number too! SHIT, only one exit code can be returned and thats column!
            }

            exit(0); // return 0 if not found!
        }
        
        else // PARENT
        {
            int status;
            int child_pid = wait(&status);

            int rowIndex = i;
            printf("Child %d (PID %d): Searching row %d", rowIndex, child_pid, rowIndex);

            if (WIFEXITED(status))
            {
                int result = WEXITSTATUS(status);

                if (result != 0)
                {
                    printf("Parent: The treasure was found by child with PID: %d at row %d and column %d", child_pid, rowIndex, result);
                }
            }  
        }
    }

    return 0;
}