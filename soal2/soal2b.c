#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>

typedef struct {
    long long *newVal;
    int matA;
    int matB;
}Args;
void display(int mat[][10], int row, int column);
void display_lld(long long mat[][10], int row, int column);
void* process(void *argument);
Args *createArgs(long long *nv, int a, int b);
unsigned long long faktorial(int num);

void main()
{
    // Shared Memory
    key_t key = 1234;
    int base[10][10];
    int (*result)[10];
    long long new[10][10];

    pthread_t tid[4][6];

    int shmid = shmget(key, sizeof(int[10][10]), IPC_CREAT | 0666);
    result = shmat(shmid, NULL, 0);

    // Debugging
    printf("-=- Matrix A -=-\n");
    display(result, 4, 6);

    printf("Input matrix B 4x6\n");
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 6; j++) {
            scanf("%d", &base[i][j]);
        }
    }
    
    printf("\n-=- Matrix B -=-\n");
    display(result, 4, 6);

    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 6; j++) {
            
            int c = 0, err;
            //membuat thread
            err=pthread_create(&tid[i][j],
                                NULL,
                                process, 
                                (void *)createArgs(&(new[i][j]),
                                                    result[i][j],
                                                    base[i][j]
                                )
                              ); 
            if(err!=0) //cek error
            {
                // printf("\n can't create thread : [%s]",strerror(err));
            }
            else
            {
                // printf("\n create thread success\n");
            }
        }
    }

    printf("-=- Matrix C -=-\n");
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 6; j++) {
            pthread_join(tid[i][j], NULL);
        }
    }    

    display_lld(new, 4, 6);
    
    shmdt(result);
    shmctl(shmid, IPC_RMID, NULL);
}

Args *createArgs(long long *nv, int a, int b) {
    Args *arg = malloc(sizeof(Args));

    arg->newVal = nv;
    arg->matA = a;
    arg->matB = b;

    return arg;
}

void* process(void *argument) {
    
    Args *arg = (Args *)argument;

    if(arg->matA == 0 | arg->matB == 0) *(arg->newVal) = 0;
    else if(arg->matA < arg->matB) *(arg->newVal) = faktorial(arg->matA);
    else *(arg->newVal) = (faktorial(arg->matA) / faktorial(arg->matA - arg->matB));
    
}

unsigned long long faktorial(int num) {
    unsigned long long temp = 1;
    printf("\n\n");
    for(int i = 1; i <= num; i++) {
        temp *= i;
    }
    
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
void display_lld(long long m[][10], int row, int column) {

   printf("\nOutput Matrix:\n");
   for (int i = 0; i < row; ++i) {
      for (int j = 0; j < column; ++j) {
         printf("%lld  ", m[i][j]);
         if (j == column - 1)
            printf("\n");
      }
   }
}