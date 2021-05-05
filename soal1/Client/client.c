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

  printf("Id : ");
  getchar();
  scanf("%[^\n]s", id);
  send(sock, id, strlen(id), 0);
  printf("Password : ");
  getchar();
  scanf("%[^\n]s", pass);
  send(sock, pass, strlen(pass), 0);

  printf("%s\n", id);
  printf("%s\n", pass);

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

  printf("🚀 code to add or delete or else here\n");

  // Prompt again
  promptChoice(sock);
}