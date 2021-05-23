#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pthread_t tid[100000];
int curIndex;
char *curDir;
char tempWrkDir[10000];
char formerDir[10000];

int threadCount = 2;

void moveFileUtil(char source[], char dest[]);
char *cleanFolderFromPath(char str[]);
void lisrecAlphaNum(char *basePath);
char *getExt(char str[]);

void *moveFile(void *arg) {
  char *fileName = (char *)arg;
  // printf("🚀 fileName: %s\n", fileName);

  char fileAsli[1000], fileCopy[1000];
  strcpy(fileAsli, fileName);
  strcpy(fileCopy, fileName);

  char *ext = getExt(fileName);
  char *cleanName = cleanFolderFromPath(fileCopy);

  // TODO 1. Make appropriate directory
  char folderName[120];
  // printf("🚀 cleanName: %s\n", cleanName);

  if (cleanName[0] == '.') {
    sprintf(folderName, "Hidden");
  } else if (ext == NULL) {
    sprintf(folderName, "Unknown");
  } else {
    // TODO lowercase file ext (JPG => jpg)
    for (int i = 0; ext[i]; i++) {
      ext[i] = tolower(ext[i]);
    }
    sprintf(folderName, "%s", ext);
  }
  mkdir(folderName, 0777);

  // TODO
  char destDir[200];
  sprintf(destDir, "%s/%s/%s", curDir, folderName,
          cleanFolderFromPath(fileAsli));
  // printf("🚀 fileAsli: %s\n", fileAsli);
  // printf("🚀 destDir: %s\n", destDir);

  moveFileUtil(fileAsli, destDir);
  return NULL;
}

int main(int argc, char **argv) {
  char buf[1000];
  curDir = getcwd(buf, 1000);

  int i = 2, p;
  int err;

  if (strcmp(argv[1], "-f") == 0) {
    // ? Incrementally check next argument
    while (argv[i] != NULL) {
      err = pthread_create(&(tid[i - 2]), NULL, &moveFile, (void *)argv[i]);
      if (err != 0)
        printf("File %d: Sad, gagal :(\n", i - 1);
      else
        printf("File %d : Berhasil Dikategorikan\n", i - 1);
      i++;
    }

    for (p = 0; p < (i - 1); p++) pthread_join(tid[p], NULL);
  } else if (strcmp(argv[1], "-d") == 0) {
    lisrecAlphaNum(argv[2]);
  } else if (strcmp(argv[1], "*") == 0) {
    lisrecAlphaNum(".");
  }

  return 0;
}

void lisrecAlphaNum(char *basePath) {
  char path[1000], srcPathForThread[1000];
  struct dirent **namelist;
  int n;
  int i = 0;
  n = scandir(basePath, &namelist, NULL, alphasort);
  if (n < 0)
    return;
  else {
    while (i < n) {
      if (strcmp(namelist[i]->d_name, ".") != 0 &&
          strcmp(namelist[i]->d_name, "..") != 0) {
        strcpy(path, basePath);
        strcat(path, "/");
        strcat(path, namelist[i]->d_name);

        strcat(srcPathForThread, "/");
        strcat(srcPathForThread, namelist[i]->d_name);

        if (namelist[i]->d_type != DT_DIR) {
          int err;
          err = pthread_create(&(tid[threadCount - 2]), NULL, &moveFile,
                               (void *)path);

          if (err != 0)
            printf("File %d: Sad, gagal :(\n", threadCount - 1);
          else
            printf("File %d : Berhasil Dikategorikan\n", threadCount - 1);
          threadCount++;

          for (int p = 0; p < (threadCount - 1); p++)
            pthread_join(tid[p], NULL);
        }

        lisrecAlphaNum(path);
      }
      free(namelist[i]);
      ++i;
    }
    free(namelist);
  }
}

void moveFileUtil(char source[], char dest[]) {
  int ch;
  FILE *fp1, *fp2;

  fp1 = fopen(source, "r");
  fp2 = fopen(dest, "w");

  if (!fp1) {
    printf("Unable to open source file to read!!\n");
    fclose(fp2);
    return;
  }

  if (!fp2) {
    printf("Unable to open target file to write\n");
    return;
  }

  while ((ch = fgetc(fp1)) != EOF) {
    fputc(ch, fp2);
  }

  fclose(fp1);
  fclose(fp2);

  // printf("%s\n", source);
  // printf("%s\n", dest);

  remove(source);
  return;
}

char *cleanFolderFromPath(char str[]) {
  char *pch;
  char *result;
  pch = strchr(str, '/');
  if (pch == NULL) return str;
  while (pch != NULL) {
    result = pch + 1;
    pch = strchr(pch + 1, '/');
  }
  return result;
}

char *getExt(char str[]) {
  char *pch = cleanFolderFromPath(str);
  // get first occurence of .
  char *result = strchr(pch, '.');
  if (result == NULL) {
    return NULL;
  } else {
    // remove the . (.txt => txt)
    return (result + 1);
  }
}