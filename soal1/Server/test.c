
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  FILE *fp;
  fp = fopen("FILES/akun.txt", "r");

  char data[1024] = {0};

  while (fgets(data, 1024, fp) != NULL) {
    printf("%s", data);
  }
}