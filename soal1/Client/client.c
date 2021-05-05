#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

void promptLogReg(int sock);
void promptChoice(int sock);
void promptAdd(int sock);
void promptDelete(int sock);
void printLogRegMsg(int id);
void send_file(FILE *fp, int sockfd);

int main(int argc, char const *argv[]) {
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;
  char *hello = "Hello from client";
  char buffer[1024] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nConnection Failed \n");
    return -1;
  }

  while (1) {
    char authMessage[1024] = {0};
    int valread;
    printf("🚀 reading for authmessage \n");
    valread = read(sock, authMessage, 1024);
    printf("%s\n", authMessage);

    if (strcmp(authMessage, "wait") != 0) {
      promptLogReg(sock);
    } else {
      printf("Waiting...\n");
    }
  }

  return 0;
}

// Prompt the client to login or register
void promptLogReg(int sock) {
  char choice[120], id[100], pass[100];
  printf("\n=====\nChoose between login or register\n> ");
  scanf("%s", choice);
  // TODO REMOVE TEMPORARY STOP
  if (strcmp(choice, "stop") == 0) {
    send(sock, "stop", strlen("stop"), 0);
    exit(0);
  }

  // if wrong, then restart the prompt.
  if (!strcmp(choice, "login") == 0 && !strcmp(choice, "register") == 0) {
    promptLogReg(sock);
    return;
  }
  // Send choice to server
  send(sock, choice, strlen(choice), 0);

  // Input for login notice
  printf("Id : ");
  getchar();
  scanf("%[^\n]s", id);
  send(sock, id, strlen(id), 0);
  printf("Password : ");
  getchar();
  scanf("%[^\n]s", pass);
  send(sock, pass, strlen(pass), 0);

  // AFTER SENDING ID AND PASSWORD, CHECK IF 1 THEN SUCCESSFULL
  printf("🚀 reading for login reg message \n");
  char logRegMsg[1024] = {0};
  int valread;
  valread = read(sock, logRegMsg, 1024);
  printf("🚀 %s\n", logRegMsg);
  if (strcmp(logRegMsg, "1") == 0) {
    printf("Login Successful 🥳\n");
  } else if (strcmp(logRegMsg, "2") == 0) {
    printf("Register Successful 🥳, you can now login\n");
    promptLogReg(sock);
  } else {
    printf("Login Failed 💔\n");
    promptLogReg(sock);
  }

  // Move to choices
  promptChoice(sock);
}

void promptChoice(int sock) {
  char choice[120], id[100], pass[100];
  printf("\n=====\nChoose between add or delete\n> ");
  scanf("%s", choice);
  // TODO REMOVE TEMPORARY STOP
  if (strcmp(choice, "stop") == 0) {
    send(sock, "stop", strlen("stop"), 0);
    exit(0);
  }

  // if wrong, then restart the prompt.
  if (!strcmp(choice, "add") == 0 && !strcmp(choice, "delete") == 0) {
    promptChoice(sock);
    return;
  }

  if (strcmp(choice, "add") == 0) {
    promptAdd(sock);
  }

  if (strcmp(choice, "delete") == 0) {
    promptDelete(sock);
  }

  // Prompt again
  promptChoice(sock);
}

void promptAdd(int sock) {
  send(sock, "add", strlen("add"), 0);

  char publisher[100], tahunPublikasi[100], filename[100];
  printf("Publisher: ");
  getchar();
  scanf("%[^\n]s", publisher);
  send(sock, publisher, strlen(publisher), 0);
  printf("Tahun Publikasi: ");
  getchar();
  scanf("%[^\n]s", tahunPublikasi);
  send(sock, tahunPublikasi, strlen(tahunPublikasi), 0);
  printf("Filepath: ");
  getchar();
  scanf("%[^\n]s", filename);
  send(sock, filename, strlen(filename), 0);
  sleep(1);

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    perror("[-]Error in reading file.");
    exit(1);
  }

  sleep(1);
  send_file(fp, sock);
  printf("[+]File data sent successfully.\n");
}

void promptDelete(int sock) {
  send(sock, "delete", strlen("delete"), 0);

  char filename[100];
  scanf("%s", filename);

  // avoid send being merged
  sleep(1);
  send(sock, filename, strlen(filename), 0);
}

void send_file(FILE *fp, int sockfd) {
  int n;
  char data[1024] = {0};

  while (fgets(data, 1024, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      perror("[-]Error in sending file.");
      exit(1);
    }
    bzero(data, 1024);
  }

  // mark last with ending
  send(sockfd, "stop signal", sizeof("stop signal"), 0);
}