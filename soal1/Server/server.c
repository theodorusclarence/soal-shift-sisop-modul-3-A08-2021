
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

int main(int argc, char const *argv[]) {
  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  printf("running on port %d\n", PORT);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // ?  INI CODE ASLI, di mac SO_REUSEPORT harus diapus
  // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
  //                sizeof(opt))) {

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t *)&addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  int cumulative = 5;

  while (1) {
    char buffer[1024] = {0};
    valread = read(new_socket, buffer, 1024);
    printf("%s\n", buffer);

    // TODO REMOVE TEMPORARY STOP
    if (strcmp(buffer, "stop") == 0) {
      char *message = "Shutting down..\n";
      send(new_socket, message, strlen(message), 0);
      break;
    }

    if (strcmp(buffer, "login") == 0 || strcmp(buffer, "register") == 0) {
      char id[1024] = {0};
      valread = read(new_socket, id, 1024);
      char password[1024] = {0};
      valread = read(new_socket, password, 1024);

      printf("id: %s, password: %s\n", id, password);

      // if register then write the id and username to the akun.txt
      if (strcmp(buffer, "register") == 0) {
        FILE *f;
        f = fopen("akun.txt", "a+");
        fprintf(f, "%s:%s\n", id, password);

        char authMessage[100];
        sprintf(authMessage, "Register Success");
        send(new_socket, authMessage, strlen(authMessage), 0);

        fclose(f);
      }

      if (strcmp(buffer, "login") == 0) {
        FILE *f;
        f = fopen("akun.txt", "a+");
        char buffer[1024] = "";
        int isValid = 0;
        // While masih ada yang bisa diread, dan selagi masih belom valid (belom
        // nemu yang id pw sama)
        while (fgets(buffer, 1024, f) != NULL && !isValid) {
          char compare_id[1025], compare_pw[1025];

          // tokenize with ':' as a delimiter
          char *token = strtok(buffer, ":");
          strcpy(compare_id, token);

          // get next token until it meets '\n'
          token = strtok(NULL, "\n");
          strcpy(compare_pw, token);

          if (strcmp(compare_id, id) == 0 &&
              strcmp(compare_pw, password) == 0) {
            isValid = 1;
          }
        }

        char authMessage[500];
        sprintf(authMessage, "validity: %d\n", isValid);
        send(new_socket, authMessage, strlen(authMessage), 0);
        fclose(f);
      }
    }
  }

  return 0;
}