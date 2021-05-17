#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<dirent.h>

pthread_t tid[100000];
int curIndex;
char *workingDir;
char tempWrkDir[10000];
char formerDir[10000];

void moveFileUtil(char source[], char dest[]);
char* getFilename(char str[]);
char* getExt(char str[]);

void* moveFile(void *arg)
{
	pthread_t id=pthread_self();
    char *extFolder;
    char destFolder[10000];
    extFolder = getExt((char *)arg);
    if(extFolder == NULL)
        strcpy(destFolder, "Unknown");
    else{
        strcpy(destFolder, extFolder);
        int i;
        for(i=0; i<strlen(destFolder); i++)
            if(destFolder[i] > 64 && destFolder[i] < 91)
                destFolder[i] += 32;
    }
            
    if(mkdir(destFolder, 0777) == -1);

    char destPath[10000];
    snprintf(destPath, 10000, "%s/%s/%s", workingDir, destFolder, getFilename((char *)arg));
    moveFileUtil((char *)arg, destPath);

	return NULL;
}

void* moveAllFile(void *arg)
{
	pthread_t id=pthread_self();
    char *extFolder;
    char destFolder[10000];
    extFolder = getExt((char *)arg);
    if(extFolder == NULL)
        strcpy(destFolder, "Unknown");
    else{
        strcpy(destFolder, extFolder);
        int i;
        for(i=0; i<strlen(destFolder); i++)
            if(destFolder[i] > 64 && destFolder[i] < 91)
                destFolder[i] += 32;
    }
            
    if(mkdir(destFolder, 0777) == -1);

    char destPath[10000];
    char sourcePath[10000];
    snprintf(sourcePath, 10000, "%s/%s", formerDir, (char *)arg);
    snprintf(destPath, 10000, "%s/%s/%s", workingDir, destFolder, getFilename((char *)arg));
    moveFileUtil(sourcePath, destPath);

	return NULL;
}

void* moveTempFile(void *arg)
{
	pthread_t id=pthread_self();
    char *extFolder;
    char destFolder[10000];
    extFolder = getExt((char *)arg);
    if(extFolder == NULL)
        strcpy(destFolder, "Unknown");
    else{
        strcpy(destFolder, extFolder);
        int i;
        for(i=0; i<strlen(destFolder); i++)
            if(destFolder[i] > 64 && destFolder[i] < 91)
                destFolder[i] += 32;
    }

    chdir(formerDir);            
    if(mkdir(destFolder, 0777) == -1);
    chdir(tempWrkDir);

    char destPath[10000];
    char sourcePath[10000];
    snprintf(sourcePath, 10000, "%s/%s", tempWrkDir, (char *)arg);
    snprintf(destPath, 10000, "%s/%s/%s", formerDir, destFolder, getFilename((char *)arg));
    printf("%s %s\n", sourcePath, destPath);
    moveFileUtil(sourcePath, destPath);

	return NULL;
}

int main(int argc, char **argb){
    char buf[1000];
    workingDir = getcwd(buf, 1000);
    snprintf(formerDir, 10000, "%s", workingDir);

    int i=2, p;
    int err;

    if(!strcmp(argb[1], "-f")){
        while(argb[i] != NULL){
            err=pthread_create(&(tid[i-2]),NULL,&moveFile,(void *)argb[i]);
            if(err!=0)
                printf("Gagal\n");
            else
                printf("Sukses\n");
            i++;
        }

        for(p=0; p<(i-1); p++)
            pthread_join(tid[p],NULL);
    } else if(!strcmp(argb[1], "*")) {
        DIR *d;
        struct dirent *dir;
        struct stat myFile;
        d = opendir(".");
        if(d){
            while((dir = readdir(d)) != NULL){
                if (stat(dir->d_name, &myFile) < 0);
                else if (!S_ISDIR(myFile.st_mode))
                {
                    err=pthread_create(&(tid[i-2]),NULL,&moveAllFile,(void *)dir->d_name);
                    if(err!=0)
                        printf("Gagal\n");
                    else
                        printf("Sukses\n");
                    i++;
                } else;
            }

            for(p=0; p<(i-1); p++)
                pthread_join(tid[p],NULL);
        }
    } else if(!strcmp(argb[1], "-d")) {
        chdir(argb[2]);
        workingDir = getcwd(buf, 1000);
        snprintf(tempWrkDir, 10000, "%s", workingDir);
        DIR *d;
        struct dirent *dir;
        struct stat myFile;
        d = opendir(".");
        if(d){
            while((dir = readdir(d)) != NULL){
                if (stat(dir->d_name, &myFile) < 0);
                else if (!S_ISDIR(myFile.st_mode))
                {
                    err=pthread_create(&(tid[i-2]),NULL,&moveTempFile,(void *)dir->d_name);
                    if(err!=0)
                        printf("Gagal\n");
                    else
                        printf("Sukses\n");
                    i++;
                } else;
            }

            for(p=0; p<(i-1); p++)
                pthread_join(tid[p],NULL);
}
    }
    return 0;
}

void moveFileUtil(char source[], char dest[]){
    int ch;
    FILE *fp1, *fp2;

    fp1 = fopen(source, "r");
    fp2 = fopen(dest, "w");
    
    if (!fp1) {
            printf("Unable to open source file to read!!\n");
            fclose(fp2);
            return ;
    }

    if (!fp2) {
            printf("Unable to open target file to write\n");
            return ;
    }

    while ((ch = fgetc(fp1)) != EOF) {
            fputc(ch, fp2);
    }

    fclose(fp1);
    fclose(fp2);
    
    //printf("%s\n", source);
    //printf("%s\n", dest);

    remove(source);
    return ;
}

char* getFilename(char str[]){
    char* pch;
    char* result;
    pch = strchr(str,'/');
    if(pch == NULL)
        return str;
    while (pch != NULL) {
        result = pch+1;
        pch = strchr(pch+1,'/');
    }
    return result;
}

char* getExt(char str[]){
    char* pch = getFilename(str);
    char* result = strchr(pch, '.');
    if(result == NULL)
        return NULL;
    else
        return (result+1);
}