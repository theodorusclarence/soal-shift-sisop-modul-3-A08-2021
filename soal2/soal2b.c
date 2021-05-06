#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>

void display(int result[][10], int row, int column);
void display_llu(unsigned long long result[][10], int row, int column);
void* process(void *arg);
unsigned long long faktorial(int num);
int base[10][10] = {{7, 8, 4, 9, 12, 20},
                       {5, 8, 9, 11, 15, 10},
                       {9, 7, 8, 10, 6, 16},
                       {6, 5, 12, 14, 6, 11}
                      };
unsigned long long new[10][10] = {{0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0}
                    };
int (*result)[10];
pthread_t tid;

void main()
{
    // Shared Memory
    key_t key = 1234;

    int shmid = shmget(key, sizeof(int[10][10]), IPC_CREAT | 0666);
    result = shmat(shmid, NULL, 0);

    // Debugging
    display(result, 4, 6);
    
    int c = 0, err; 
    err=pthread_create(&tid,NULL,&process, NULL); //membuat thread
    if(err!=0) //cek error
    {
        printf("\n can't create thread : [%s]",strerror(err));
    }
    else
    {
        printf("\n create thread success\n");
    }
    pthread_join(tid, NULL);
    

    display_llu(new, 4, 6);
    
    shmdt(result);
    shmctl(shmid, IPC_RMID, NULL);
}

void* process(void *arg) {
    // display(result, 4, 6);
    // display(base, 4, 6);
    // display(new, 4, 6);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 6; j++)
        {   
            if(result[i][j] == 0 | base[i][j] == 0) new[i][j] = 0;
            else if(result[i][j] < base[i][j]) new[i][j] = faktorial(result[i][j]);
            else new[i][j] = (faktorial(result[i][j]) / faktorial(result[i][j]-base[i][j]));
            printf("\n%d-%d=%llu", result[i][j], base[i][j], new[i][j]);
        }
        
    }
    // display(new, 4, 6);
}

unsigned long long faktorial(int num) {
    unsigned long long temp = 1;
    printf("\n\n");
    for(int i = 1; i <= num; i++) {
        printf("%llu->", temp);
        temp *= i;
    }
    // while(num > 0) {
    //     printf("%d->", temp);
    //     temp *= num;
    //     num--;
    // }
    printf("(%llu)", temp);
    return temp;
}

// function to display the matrix
void display(int m[][10], int row, int column) {

   printf("\nOutput Matrix:\n");
   for (int i = 0; i < row; ++i) {
      for (int j = 0; j < column; ++j) {
         printf("%d  ", m[i][j]);
         if (j == column - 1)
            printf("\n");
      }
   }
}

// function to display the matrix
void display_llu(unsigned long long m[][10], int row, int column) {

   printf("\nOutput Matrix:\n");
   for (int i = 0; i < row; ++i) {
      for (int j = 0; j < column; ++j) {
         printf("%llu  ", m[i][j]);
         if (j == column - 1)
            printf("\n");
      }
   }
}