#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <limits.h>

pthread_t tid[500]; //max 500 thread
typedef struct arg_struct {
    char asal[1000];
    char cwd[1000];
}arg_struct;

int is_regular_file(const char *path) //jika 0 bukan file
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void pindahFile(char *argv, char *cwd){
//   printf("stringvoid = %s\n", argv);
//   printf("stringvoid = %s\n", cwd);
  
  char string[1000];
  strcpy(string, argv);
  int isFile = is_regular_file(string);
  char dot = '.'; 
  char slash = '/';
  char* tipe = strrchr(string, dot); 
  char* nama = strrchr(string, slash);
  
  char tipeLow[100];
  if(tipe)
  {
    if(tipe[strlen(tipe)-1] >= '*' && tipe[strlen(tipe)-1] <= '~')
    {
      strcpy(tipeLow, tipe);
      for(int i = 0; tipeLow[i]; i++){
        tipeLow[i] = tolower(tipeLow[i]);
      }
    }
  }
  else
  {
  strcpy(tipeLow, " Unknown"); //tanpa ekstensi
  }
    mkdir((tipeLow + 1), 0777); //bikin folder ekstensi

    size_t len = 0 ;
    // strcpy
    char a[1000] ; //res
    strcpy(a, argv);
    char b[1000] ; //des
    strcpy(b, cwd);
    strcat(b, "/");
    strcat(b, tipeLow+1);
    strcat(b, nama);
    rename(a, b);
    remove(a);
}

void *pindahf(void* arg){
  arg_struct args = *(arg_struct*) arg;
//   printf("stringthr = %s\n", args.asal);
  // printf("stringthr = %s\n", args.cwd);
  pindahFile(args.asal, args.cwd);
  pthread_exit(0);
}

void sortHere(char *asal){
  arg_struct args;
  // args.cwd = "/home/yusuf/soal-shift-sisop-modul-3-A08-2021/soal3";
  strcpy(args.cwd,"/home/yusuf/soal-shift-sisop-modul-3-A08-2021/soal3");
  DIR *dirp;
    struct dirent *entry;
    dirp = opendir(asal);
    int index = 0;
    while((entry = readdir(dirp)) != NULL)
    {
      if(entry->d_type == DT_REG)
      {
        char namafile[105];
        sprintf(namafile, "%s/%s", asal, entry->d_name);
        strcpy(args.asal, namafile);
        if(strcmp(namafile, "/home/yusuf/soal-shift-sisop-modul-3-A08-2021/soal3/soal3.c")!=0)
        {
            pthread_create(&tid[index], NULL, pindahf, (void *)&args);
            sleep(1);
            index++; 
        }
      }
    }
}
int main(int argc, char* argv[]) 
{ 

  // char cwd[1000];
  arg_struct args;
  getcwd(args.cwd, sizeof(args.cwd));

  
  if(strcmp(argv[1],"-f")==0)
  {
    int index = 0;
    for (int i = 2; i < argc; i++)
    {
      strcpy(args.asal, argv[i]);
      pthread_create(&tid[index], NULL, pindahf, (void *)&args);
      index++;
      printf("File %d : Berhasil Dikategorikan\n",index);
    }
    for (int i = 0; i < index; i++) {
        pthread_join(tid[i], NULL);
    }
  }
  else if(strcmp(argv[1],"*")==0)
  {
    char asal[] = "/home/yusuf/soal-shift-sisop-modul-3-A08-2021/soal3";
    
    sortHere(asal);
  }
  else if(strcmp(argv[1],"-d")==0){
      char asal[1000];
      strcpy(asal, argv[2]);
      sortHere(asal);
    //   rename(asal, args.cwd);
    //   sortHere(asal);
    printf("Direktori sukses disimpan!\n");
  }
  else
  {
      printf("MAAF SALAH\n");
      return 0;
  }
  
	return 0; 
} 