
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  printf("hi\n");
  FILE *f;
  f = fopen("akun.txt", "a+");

  char buffer[1024];
  int isValid = 0;
  while (fgets(buffer, 1024, f) != NULL && !isValid) {
    char username[100], password[100];

    // tokenize with ':' as a delimiter
    char *token = strtok(buffer, ":");
    strcpy(username, token);

    // get next token until it meets '\n'
    token = strtok(NULL, "\n");
    strcpy(password, token);
    printf("--%s\n", username);
    printf("--%s\n", password);

    if (strcmp(username, "clarence") == 0 || strcmp(password, "bambang") == 0) {
      isValid = 1;
    }
  }

  printf("valid: %d\n", isValid);
}