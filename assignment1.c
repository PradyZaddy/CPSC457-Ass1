#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *args[]) {

  // declaring out treasure Matrix
  int row = 100;
  int column = 1000;
  int treasureMatrix[100][1000];

  // reading input
  for (int i = 0; i < row; i++) {
    for (int j = 0; j < column; j++) {
      if (scanf("%d", &treasureMatrix[i][j]) != 1) {
        fprintf(stderr, "Error: Not enough input data.\n");
        exit(1);
      }
    }
  }

  pid_t rcArray[100];

  // fork() is called for each number of row; n(processes) = n(row)
  for (int i = 0; i < row; i++) {
    pid_t rc = fork();

    if (rc == 0) // CHILD
    {
      int found = 0;

      for (int j = 0; j < column; j++) {

        // if the treasure is found, the child returns 1 as it's exit code
        if (treasureMatrix[i][j] == 1) {
          found = 1;
          exit(found);
        }
      }

      exit(found); // return 0 if not found!
    }

    // PARENT PROCESS - if (pid > 0), we store the pid of each processes into
    // the rcArray for reference later.
    else {
      rcArray[i] = rc; // storing the pid of the child processes
      printf("Child %d (PID: %d): Searching row %d \n", i, rcArray[i], i);
    }
  }

  int treasure_found = 0;

  // running a for loop that waits for all the child processes to finish first.
  for (int k = 0; k < row; k++) {
    int status;
    pid_t finished = wait(&status);

    // if the process exits successfully, we receive the 1 and stored in found.
    if (WIFEXITED(status)) {
      int found = WEXITSTATUS(status);

      // as we know about the row number, we go to the column to find the exact
      // lcoation of the treasure.
      if (found == 1) {

        treasure_found = 1;

        for (int r = 0; r < row; r++) {
          if (rcArray[r] == finished) {
            int found_col = -1;

            for (int c = 0; c < column; c++) {
              if (treasureMatrix[r][c] == 1) {
                found_col = c;
                break;
              }
            }
            printf("Treasure found by child with PID %d at row %d, col %d \n",
                   finished, found_col, r);
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
