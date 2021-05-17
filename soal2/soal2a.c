#include <stdio.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

void multiplyMatrices(int first[][10],
                      int second[][10],
                      int result[][10],
                      int r1, int c1, int r2, int c2);
void display(int result[][10], int row, int column);

int main() {
   // Shared Memory
    key_t key = 1234;
    int (*result)[10];

    int shmid = shmget(key, sizeof(int[10][10]), IPC_CREAT | 0666);
    result = shmat(shmid, NULL, 0);
    
    // Multiply Matrices
    int r1, c1, r2, c2;
    int first[10][10];
    int second[10][10];
   
    r1 = 4;
    c1 = 3;
    r2 = 3;
    c2 = 6;

   printf("Input matrix 4x3\n");
   for(int i = 0; i < r1; i++) {
      for(int j = 0; j < c1; j++) {
         scanf("%d", &first[i][j]);
      }
   }

   printf("Input matrix 3x6\n");
   for(int i = 0; i < r2; i++) {
      for(int j = 0; j < c2; j++) {
         scanf("%d", &second[i][j]);
      }
   }

   // Taking input until
   // 1st matrix columns is not equal to 2nd matrix row
   while (c1 != r2) {
      printf("Error! Enter rows and columns again.\n");
      return 0;
   }

   // multiply two matrices.
   
   multiplyMatrices(first, second, result, r1, c1, r2, c2);

   // display the result
   display(result, r1, c2);

    sleep(5);
    printf("\nNungguin ya?\n\n");
    sleep(20);
    shmdt(result);
    shmctl(shmid, IPC_RMID, NULL);

   return 0;
}

// function to multiply two matrices
void multiplyMatrices(int first[][10],
                      int second[][10],
                      int result[][10],
                      int r1, int c1, int r2, int c2) {

   // Initializing elements of matrix mult to 0.
   for (int i = 0; i < r1; ++i) {
      for (int j = 0; j < c2; ++j) {
         result[i][j] = 0;
      }
   }
   // Multiplying first and second matrices and storing it in result
   for (int i = 0; i < r1; ++i) {
      for (int j = 0; j < c2; ++j) {
         for (int k = 0; k < c1; ++k) {
            result[i][j] += first[i][k] * second[k][j];
         }
      }
   }
}

// function to display the matrix
void display(int result[][10], int row, int column) {

   printf("\nOutput Matrix:\n");
   for (int i = 0; i < row; ++i) {
      for (int j = 0; j < column; ++j) {
         printf("%d  ", result[i][j]);
         if (j == column - 1)
            printf("\n");
      }
   }
}