#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main (int argc, char *args[])
{

    int row = 100;
    int column = 1000;
    int treasureMatrix[100][1000];

    FILE *fp = fopen("test6.txt", "r");   // open file for reading
    if (fp == NULL) 
    {
        perror("Error opening file");
        return 1;
    }

    // read all numbers into the matrix
    for (int i = 0; i < row; i++) 
    {
        for (int j = 0; j < column; j++) 
        {
            fscanf(fp, "%d", &treasureMatrix[i][j]);
        }
    }

    fclose(fp);

    pid_t rcArray[100];


    for (int i = 0; i < row; i++)
    {
        pid_t rc = fork();

        if (rc == 0) // CHILD 
        {
            int found = 0;

            for (int j = 0; j < column; j++)
            {

                if (treasureMatrix[i][j] == 1)
                {
                    found = 1;
                    exit(found); // return the column where the treasure is found // CANT'T DO THIS!
                }

                // exit(i); CAN'T DO THIS!, only one exit code can be returned and thats column!
            }

            exit(found); // return 0 if not found!
        }
        
        else
        {
            rcArray[i] = rc; // storing the pid of the child processes
            printf("Child %d (PID: %d): Searching row %d \n", i, rcArray[i], i);

        }
    }

    int treasure_found = 0;

    for (int k = 0; k < row; k++) {
        int status;
        pid_t finished = wait(&status);

        if (WIFEXITED(status)) {
            int found = WEXITSTATUS(status);

            if (found == 1) 
            { // treasure found
                treasure_found = 1;
                for (int r = 0; r < row; r++) 
                {
                    if (rcArray[r] == finished) 
                    {
                        int found_col = -1; 
                        for (int c = 0; c < column; c++) 
                        {
                            if (treasureMatrix[r][c] == 1) 
                            {
                                found_col = c;
                                break;
                            }
                        }
                        printf("Treasure found at row %d, col %d by child PID %d\n",
                            r, found_col, finished);
                    }
                }
            }
        }
    }

    // after all children are reaped
    if (!treasure_found) {
        printf("Parent: No treasure found in this matrix\n");
    }

    return 0;
}

