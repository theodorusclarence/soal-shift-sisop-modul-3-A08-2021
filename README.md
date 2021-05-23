# soal-shift-sisop-modul-3-A08-2021

# Soal 1
Karena soal ini berupa socket, maka potongan code yang wajib ada adalah berupa boilerplate untuk client dan server.
```c
// server.c
int main() {
  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  printf("running on port %d\n", PORT);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

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
  
  // ...other code
}
```
Boilerplate ini berfungsi untuk membuat server socket pada port yang telah ditentukan yaitu port 8000.

Ada pula boilerplate untuk client sebagai berikut:
```c
// client.c
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

  // ...other code
}
```
Boilerplate ini berfungsi untuk membuat koneksi ke server pada port yang telah ditentukan yaitu port 8000.

### 1a
> Pada soal ini, program diminta untuk bisa menyediakan layanan register dan login, dan menyimpan data" ke dalam database berupa file `akun.txt` dengan format sebagai berikut:
```
id:password
id2:password2
```
> Selain itu, soal juga meminta untuk bisa melakukan multiconnection, dan ketika ada lebih dari 1 client, maka client lain harus menunggu sampai client yang terlebih dahulu terkoneksi selesai.

Approach yang dilakukan untuk ini adalah dengan membuat prompt.

---

#### Penjelasan Multiconnection

```c
// server.c
int main() {
    // ...boilerplate
while (1) {
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }
    id_socket[currentConnection] = new_socket;

    if (connection > 0) {
      printf("📩 sending wait signal to currentConnection: %d\n",
             currentConnection);
      send(id_socket[currentConnection], "wait", strlen("wait"), 1024);

      pthread_create(&tid[currentConnection], NULL, handleLogReg, &new_socket);
    } else {
      printf("📩 sending go signal to currentConnection: %d\n",
             currentConnection);
      send(id_socket[currentConnection], "go", strlen("go"), 1024);

      pthread_create(&tid[currentConnection], NULL, handleLogReg, &new_socket);
    }
    connection++;
    currentConnection++;
  }
}
```
Untuk menghandle multiconnection, maka kami menggunakan thread untuk setiap client yang masuk, kami juga menyimpan variable `connection` untuk mengakumulasi jumlah value koneksi yang ada, dan menyimpan `currentConnection` yang berupa counter koneksi yang masuk, untuk menandai `id_socket` sehingga kita bisa mengirim pesan spesifik ke id socket tersebut nantinya.

Untuk menandakan jika sudah bisa masuk

Kemudian pada code di client, 
```c
// client.c
int main() {
    // ...boilerplate
    while (1) {
        char authMessage[1024] = {0};
        int valread;
        printf("🚀🕓 reading for authmessage \n");
        valread = read(sock, authMessage, 1024);
        printf("🚀 authMessage: %s\n", authMessage);

        if (strcmp(authMessage, "wait") != 0) {
        promptLogReg(sock);
        } else {
        printf("Waiting...\n");
        }
    }
}
```

yang dilakukan pertama kali saat berjalan adalah dengan menunggu read, dan read yang harus dikirim adalah berupa selain 'wait', jika yang didapat selain 'wait' maka akan dikirim ke `promptLogReg()`, jika masih 'wait', maka code akan melakukan looping dan menunggu read kembali.

Pada server, dilakukan pengecekan apabila connection (yang berarti jumlah ril berapa koneksi yang ada) lebih dari 1, maka akan mengirimkan signal 'wait', untuk memberikan kode untuk client menunggu karena sudah ada client yang terlebih dahulu berjalan.

![Running](https://i.imgur.com/Xj2JOXx.png)


---

#### Penjelasan Login & Register
Kemudian untuk login & register, ada function baik di server dan di client yang dijalankan untuk saling berkomunikasi kebutuhan yang harus dikirim dan diterima

```c
//server.c

void *handleLogReg(void *args) {
  int new_socket = *(int *)args;
  int valread;

  char buffer[1024] = {0};
  valread = read(new_socket, buffer, 1024);
  printf("%s\n", buffer);

  // TODO REMOVE TEMPORARY STOP
  if (strcmp(buffer, "stop") == 0) {
    printf(
        "🚀 [handleLogReg()] just got stop signal, proceeding "
        "handleStopConnection()\n");
    handleStopConnection(new_socket);
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
      sprintf(authMessage, "2");
      send(new_socket, authMessage, strlen(authMessage), 0);

      fclose(f);

      handleLogReg(&new_socket);
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

        if (strcmp(compare_id, id) == 0 && strcmp(compare_pw, password) == 0) {
          isValid = 1;
        }
      }

      char authMessage[500];
      sprintf(authMessage, "%d", isValid);
      send(new_socket, authMessage, strlen(authMessage), 0);
      fclose(f);

      if (isValid) {
        handleSecondPhase(new_socket, id, password);
      } else {
        handleLogReg(&new_socket);
      }
    }
  } else {
    handleLogReg(&new_socket);
    pthread_cancel(pthread_self());
  }
}
```
Pada server, yang read adalah command yang dikirimkan oleh client. Client dapat mengirimkan 3 command yang akan diproses oleh server yaitu: `stop`, `login`, dan `register`.

Client mengirimkan value" login dengan function sebagai berikut:
```c
//client.c
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
  printf("🚀🕓 reading for logReg message \n");
  char logRegMsg[1024] = {0};
  int valread;
  valread = read(sock, logRegMsg, 1024);
  printf("🚀 logRegMst: %s\n", logRegMsg);

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
```

Client mengirimkan id, password, dan menunggu feedback dari server.

1. Pada kasus `register`, Server membuat sebuah file dengan read file 'a+' yang berarti append, dan membuat file apabila file belum tersedia. yang dilakukkan adalah langsung meng-append ke file `akun.txt` sesuai format yang diberikan. Kemudian server akan memberikan kode feedback ke client.
2. Pada kasus `login`, server akan membaca file `akun.txt` dengan command yang sama, dan melakukan pembacaan file line by line. Line yang dibaca itu kemudian di tokenize menggunakan strtok, dan kita bisa mengekstrak variable id dan passwordnya. yang perlu dilakukan adalah jika ketemu, maka mengatur flag `isValid` ke 1, dan menyetop looping read line, dan mengembalikan kode sukses dan mengarahkan ke secondPhase. Ketika tidak ketemu login info yang benar, maka akan mengirim kode gagal, dan melakukan `handleLogReg` ulang untuk menunggu client mengirim command selanjutnya.
3. Pada kasus `stop`, ini berarti client ingin keluar dari koneksi tersebut, untuk proses penyetopan, maka dilakukan fungsi `handleStopConnection`.
```c
// server.c
void handleStopConnection(int sock) {
  connection--;

  // ambil connection sebelumnya
  // currentConnection--;
  // go to next connection in queue
  queue++;

  send(id_socket[queue], "go", strlen("go"), 1024);

  // Reset count connection and use the available tid;
  if (queue == currentConnection) {
    queue = 0;
    currentConnection = 0;
    printf("🐮🕓 currentConnection:  %d\n", currentConnection);
    printf("🐮🕓 queue:  %d\n", queue);
  }

  pthread_cancel(pthread_self());
}
```
Pada handlesStopConnection, yang dilakukan adalah dengan mengurangi variable `connection` dan `currentConnection`, kemudian mengirim 'go' ke client berikutnya yang ada di dalam antrian. Variable `queue` disimpan untuk mengirimkan message ke id_socket selanjutnya. Dan ketika `queue == currentConnection` atau dalam kata lain, semua koneksi sudah selesai semua, maka akan ada peresettan `queue` dan `currentConnection` untuk menghindari segmentation fault pada array thread yang dibuat. Dan terakhir mematikkan thread yang telah selesai digunakan.

![Foto Demo](https://i.imgur.com/GUsssjT.png)

Setelah register, data tersimpan di akun.txt
![akuntxt](https://i.imgur.com/NqrhXRi.png)

### 1b
> Sistem memiliki database `files.tsv`, dan folder `FILES` yang dibuat saat server berjalan.

```c
// server.c
mkdir("FILES", 0777);
```

Dengan menggunakan potongan code diatas, maka folder akan terbuat secara otomatis pada saat dibuat. Pembuatan files.tsv akan dijelaskan pada command add nanti.

### 1c
> Fungsi: `add`, mengirim file dari Client ke Server.

```c
// server.c
void handleSecondPhase(int sock, char *id, char *password) {
    if (strcmp(buffer, "add") == 0) {
        printf("🚀 [handleSecondPhase()] add signal\n");
        printf(
            "🚀  [handleSecondPhase()] Preserve id:password for running.log%s:%s\n",
            id, password);

        // TODO 1. Valread file name dll
        char publikasi[120] = {0};
        valread = read(sock, publikasi, 1024);
        char tahunPublikasi[120] = {0};
        valread = read(sock, tahunPublikasi, 1024);
        char filename[120] = {0};
        valread = read(sock, filename, 1024);

        // TODO 2. Write to files.tsv
        FILE *fp;
        fp = fopen("files.tsv", "a+");
        fprintf(fp, "%s\t%s\t%s\n", publikasi, tahunPublikasi, filename);
        fclose(fp);

        // TODO 3. Write to running.log
        fp = fopen("running.log", "a+");
        // FILES/ get cut off
        char *filenameWithoutFolder = filename + 6;
        fprintf(fp, "Tambah: %s (%s:%s)\n", filenameWithoutFolder, id, password);
        fclose(fp);

        // TODO 4. Write file yang dikirim by line oleh client
        write_file(sock, filename);
        printf("🥳 Data written in the file successfully.\n");
    }
}
```
Pada server, hal yang dilakukan adalah dengan me-read data" yang dikirimkan oleh client, kemudian menuliskan ke `files.tsv` dengan command 'a+', dengan format `publikasi\ttahunPublikasi\tfilename\n`.

Kemudian, setelah berhasil mendaftarkan ke database, kita menulis log sesuai format, dan menjalankan fungsi `write_file()`

```c
// server.c
void write_file(int sockfd, char *filename) {
  int n;
  FILE *fp;
  char buffer[SIZE];

  printf("🚀 [write_file] File to be written in server: %s\n", filename);

  fp = fopen(filename, "w");
  bzero(buffer, SIZE);
  while (1) {
    n = recv(sockfd, buffer, SIZE, 0);

    // Kalo yang ngirim bukan dari send_file (karena dari function send_file
    // pasti 1024)
    if (n != 1024) {
      break;
      return;
    }

    // masukkin ke filenya
    fprintf(fp, "%s", buffer);
    bzero(buffer, SIZE);
  }
  fclose(fp);
  return;
}
```

Fungsi ini digunakan untuk membaca message yang dikirimkan line by line oleh client menggunakan command `recv`, kemudian melakukan write ke file yang dibuat pada folder Server. Kemudian ditandai dengan return n yang != 1024, karena client memberikan tanda ketika sudah selesai mengirim file.

Pada Client, fungsi mengirim sebagai berikut: 
```c
// client.c
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
  printf("🥳 File data sent successfully.\n");
}
```

Fungsi ini untuk mengirimkan data" yang dibutuhkan, kemudian menjalankan fungsi `send_file` untuk mengirim file. Penggunaan sleep(1) digunakan untuk menghindari message terkirim bersamaan dengan yang sebelumnya.

```c
// client.c
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
```

Fungsi `send_file` digunakan untuk membaca file yang ingin dikirim, kemudian mengirimkan line by line berupa message ke server, dan terakhir jika sudah selesai mengirim seluruh file, maka mengirimkan "stop signal" untuk menandakan bahwa isi file sudah habis.

![Demo Add](https://i.imgur.com/DUn7oPm.png)

## 1d
> Fungsi: `download`, mengunduh file dari server ke client

Proses yang dilakukan untuk fungsi download kebalikan dari fungsi add

```c
// server.c
if (strcmp(buffer, "download") == 0) {
    // Get the filename
    char downloadFileName[120] = {0};
    valread = read(sock, downloadFileName, 1024);
    printf("🚀 [download] downloadFileName: %s\n", downloadFileName);

    char fullPathFileName[150];
    sprintf(fullPathFileName, "FILES/%s", downloadFileName);

    sleep(1);
    FILE *fp = fopen(fullPathFileName, "r");
    send_file(fp, sock);
    printf("🥳 File data sent successfully.\n");
  }
```

```c
// client.c
void promptDownload(int sock) {
  send(sock, "download", strlen("download"), 0);

  char filename[100];
  scanf("%s", filename);

  // avoid send being merged
  sleep(1);
  send(sock, filename, strlen(filename), 0);

  char fullPathFileName[150];
  sprintf(fullPathFileName, "FILES/%s", filename);

  write_file(sock, fullPathFileName);
  printf("🥳 File data downloaded successfully.\n");
}
```
Perbedaannya hanya di filePath, menggunakan filename, maka kita harus menyesuaikan pathnya terlebih dahulu.

![Demo Download](https://i.imgur.com/aFSEHfC.png)

## 1e
> Fungsi: `delete`, menghapus dari `files.tsv`, kemudian merename nama menjadi `old-nama.ext`

```c
// server.c
if (strcmp(buffer, "delete") == 0) {
    // Get the filename
    char deleteFileName[120] = {0};
    valread = read(sock, deleteFileName, 1024);
    printf("🚀 [delete] deleteFileName: %s\n", deleteFileName);

    // TODO 1. Delete from the tsv file
    FILE *fp, *fp2;
    fp = fopen("files.tsv", "r+");
    fp2 = fopen("temp.tsv", "w");

    char data[1024] = {0};
    char publisher[100], tahunPublikasi[100], filename[100];

    while (fgets(data, 1024, fp) != NULL) {
      // Read data from files.tsv
      sscanf(data, "%[^\t]\t%s\t%s", publisher, tahunPublikasi, filename);

      // FILES/ get cut off
      char *filenameWithoutFolder = filename + 6;

      if (strcmp(filenameWithoutFolder, deleteFileName) != 0) {
        // Copy if not the data that want to be deleted
        fprintf(fp2, "%s", data);
      }

      bzero(data, 1024);
    }
    fclose(fp);
    fclose(fp2);
    remove("files.tsv");
    rename("temp.tsv", "files.tsv");

    // TODO 2. Add to running log
    fp = fopen("running.log", "a+");

    // FILES/ get cut off
    fprintf(fp, "Hapus: %s (%s:%s)\n", deleteFileName, id, password);
    fclose(fp);

    // TODO 3. Change the real filename to old-
    // FILES/satu.txt
    char fullPathFileName[200];
    sprintf(fullPathFileName, "FILES/%s", deleteFileName);
    // FILES/satu.txt
    char deletedPathFileName[200];
    sprintf(deletedPathFileName, "FILES/old-%s", deleteFileName);

    rename(fullPathFileName, deletedPathFileName);
  }
```

Untuk penghapusan, yang pertama kali dilakukan adalah dengan menghapus data dari `files.tsv`, karena tidak memungkinkan untuk menghapus baris spesifik di `files.tsv`, maka yang bisa kita lakukan adalah dengan mencopy files.tsv ke file temp, dengan catatan, tidak mencopy file yang ingin kita hapus. Pengcopyan dilakukan dengan menulis baris per baris, dan melakukan pengecekan apabila nama file yang di baris tersebut sama dengan baris yang kita ingin hapus / tidak copy.

Kemudian setelah selesai mengcopy, `files.tsv` dihapus, dan `temp.tsv` direname sebagai `files.tsv`

Setelah itu kita juga melakukan pencatatan log seperti add, id dan password didapat dari parameter function.

Terakhir, yang dilakukan adalah merename dengan prefix `old-` menggunakan fungsi rename.

Untuk client, code yang dilakukan cukup simple, yaitu hanya mengirimkan nama file yang ingin didelete ke server.
```c
// client.c
void promptDelete(int sock) {
  send(sock, "delete", strlen("delete"), 0);

  char filename[100];
  scanf("%s", filename);

  // avoid send being merged
  sleep(1);
  send(sock, filename, strlen(filename), 0);
  printf("🥳 Deleted successfully.\n");
}
```

![Demo Delete](https://i.imgur.com/L9ygM6H.png)

## 1f
> Fungsi `see`, melihat semua file di `files.tsv` dengan format yang telah ditentukan

```c
// server.c
if (strcmp(buffer, "see") == 0) {
    FILE *fp;
    fp = fopen("files.tsv", "r+");

    char data[1024] = {0};
    char publisher[100], tahunPublikasi[100], filename[100];

    // To store full data that want to be sent
    char fullData[100000];

    // TODO 1. Read line by line in files.tsv
    while (fgets(data, 1024, fp) != NULL) {
        // Read data from tsv according to format
        sscanf(data, "%[^\t]\t%s\t%s", publisher, tahunPublikasi, filename);
        char line1[200], line2[200], line3[200], line4[200], line5[200];

        sprintf(line2, "Publisher: %s\n", publisher);
        sprintf(line3, "Tahun publishing: %s\n", tahunPublikasi);
        sprintf(line5, "Filepath : %s\n", filename);

        // Get file extension (strrchr returns .txt, then add+1 to remove the .)
        char *ext = strrchr(filename, '.');
        char *extension = ext + 1;
        sprintf(line4, "Ekstensi File : %s\n", extension);

        // get filename, then remove extension and path in front
        char fullPathWithoutExt[100];
        strcpy(fullPathWithoutExt, filename);
        char cleanName[100];
        fullPathWithoutExt[strlen(fullPathWithoutExt) - strlen(ext)] = '\0';
        sscanf(fullPathWithoutExt, "FILES/%s", cleanName);
        sprintf(line1, "Nama: %s\n", cleanName);

        // TODO 2. Get the processed data to fullData
        strcat(fullData, line1);
        strcat(fullData, line2);
        strcat(fullData, line3);
        strcat(fullData, line4);
        strcat(fullData, line5);
        strcat(fullData, "\n");
    }

    // TODO 3. Send the full data
    send(sock, fullData, strlen(fullData), 0);
    }
}
```
Untuk fungsi ini, sebenarnya cukup simple, tetapi lumayan banyak data yang harus diproses, approach yang kami ambil adalah dengan membaca line di `files.tsv` kemudian memprosesnya menjadi data yang siap dikirim ke dalam 1 variable string. 

Operasi yang dilakukan seperti yang sudah dijelaskan di comment, seperti mendapatkan extensi, dan filename bersih. Kemudian dikirim ke client sebagai 1 message.

Pada Client, code yang dilakukan hanya menerima message
```c
// client.c
void promptSee(int sock) {
  // TODO send see signal
  send(sock, "see", strlen("see"), 0);

  // TODO get the message then print
  char buffer[100000] = {0};
  int valread;
  valread = read(sock, buffer, 1024);
  // Print result
  printf("%s\n", buffer);
}
```

![Demo See](https://i.imgur.com/YxwgA3Y.png)

## 1g
> Fungsi: `find`, menampilkan mirip seperti `see`, tetapi hanya keyword yang cocok saja.

```c
// server.c
 if (strcmp(buffer, "find") == 0) {
    // Get the filename
    char fileNameToFind[120] = {0};
    valread = read(sock, fileNameToFind, 1024);
    printf("🚀[find] fileNameToFind: %s\n", fileNameToFind);

    FILE *fp;
    fp = fopen("files.tsv", "r+");

    char data[1024] = {0};
    char publisher[100], tahunPublikasi[100], filename[100];

    char fullData[100000];

    while (fgets(data, 1024, fp) != NULL) {
      sscanf(data, "%[^\t]\t%s\t%s", publisher, tahunPublikasi, filename);
      char line1[200], line2[200], line3[200], line4[200], line5[200];
      sprintf(line2, "Publisher: %s\n", publisher);
      sprintf(line3, "Tahun publishing: %s\n", tahunPublikasi);
      sprintf(line5, "Filepath : %s\n", filename);

      char *ext = strrchr(filename, '.');
      char *extension = ext + 1;
      sprintf(line4, "Ekstensi File : %s\n", extension);

      char fullPathWithoutExt[100];
      strcpy(fullPathWithoutExt, filename);
      char cleanName[100];
      fullPathWithoutExt[strlen(fullPathWithoutExt) - strlen(ext)] = '\0';
      sscanf(fullPathWithoutExt, "FILES/%s", cleanName);
      sprintf(line1, "Nama: %s\n", cleanName);

      if (strstr(cleanName, fileNameToFind) != 0) {
        strcat(fullData, line1);
        strcat(fullData, line2);
        strcat(fullData, line3);
        strcat(fullData, line4);
        strcat(fullData, line5);
        strcat(fullData, "\n");
      }
    }
    if (strlen(fullData) == 0) {
      send(sock, "😢 no file found", strlen("😢 no file found"), 0);
    } else {
      // If found, then return the fullData
      send(sock, fullData, strlen(fullData), 0);
    }
}
```

Pada fungsi find, mirip seperti see, tetapi yang kita print hanya filename yang cocok dengan keyword. Pengecekan dilakukan dengan code `strstr(cleanName, fileNameToFind) != 0`, jika cocok maka file tersebut dikirim, jika tidak, maka tidak perlu dikirim. Ada juga error handling ketika tidak ada file yang ketemu.

Pada Client, yang perlu dilakukan adalah mengirim keyword dan membaca output
```c
// client.c
void promptFind(int sock) {
  // TODO send the find signal
  send(sock, "find", strlen("find"), 0);

  char filename[100];
  scanf("%s", filename);

  // avoid send being merged
  sleep(1);
  send(sock, filename, strlen(filename), 0);

  // TODO Get the message from the server, then print
  char buffer[100000] = {0};
  int valread;
  valread = read(sock, buffer, 1024);
  printf("%s\n", buffer);
}
```

![Demo Find](https://i.imgur.com/ZrvVns5.png)
Dapat dilihat, file yang bernama 'lainsendiri.txt' tidak diprint sebagai output karena tidak sesuai keyword.

## 1h
> Pencatatan running.log

Pencatatan ini dilakukan pada proses add dan delete

```c
// server.c
// Write to running.log (Fungsi Add)
fp = fopen("running.log", "a+");
// FILES/ get cut off
char *filenameWithoutFolder = filename + 6;
fprintf(fp, "Tambah: %s (%s:%s)\n", filenameWithoutFolder, id, password);
fclose(fp);

// Add to running log (Fungsi Delete)
fp = fopen("running.log", "a+");
// FILES/ get cut off
fprintf(fp, "Hapus: %s (%s:%s)\n", deleteFileName, id, password);
fclose(fp);
```

![Demo Running](https://i.imgur.com/3qmaWL2.png)

## Kendala Pada Nomor 1
1. Pada saat mengirim file, sedikit kesulitan karena tidak ada tanda bahwa file telah habis, dan jadinya server menunggu read terus menerus tanpa stop. Solusinya adalah dengan mengirimkan buffer yang sizenya bukan 1024, sehingga bisa dicek jika sudah habis
![Ilustrasi menunggu](https://i.imgur.com/mc1zkQp.png)
2. Beberapa send message kadang bisa melebur hanya menjadi 1 send message, sehingga menyebabkan data yang dikirim tidak sesuai, dan server tetep menunggu read. Solusi yang diselesaikan adalah dengan memberikan `sleep(1)` untuk memberikan jeda sehingga tidak ada yang error

--- 

# Soal 2

Pada soal kita diminta untuk melakukan perkalian matriks, mengirimkan nilai ke program/proses lain, dan melihat 5 proses teratas yang sedang berjalan 

## Soal 2a
Pada soal ini kita akan melakukan 2 hal, yaitu perhitungan perkalian matriks dan pengiriman nilai (server)
### Perhitungan nilai matriks
1. Melakukan input nilai 2 matriks
```c
printf("Input matrix 4x3\n");
for(int i = 0; i < r1; i++) {
  for(int j = 0; j < c1; j++) {
      scanf("%d", &first[i][j]);
  }
}

printf("Input matrix 3x6\n");
for(int i = 0; i < r2; i++) {
  for(int j = 0; j < c2; j++) {
      scanf("%d", &second[i][j]);
  }
}
```
2. Melakukan pemanggilan fungsi `multiplyMatrices()`
```c
multiplyMatrices(first, second, result, r1, c1, r2, c2);
```
3. Didalam fungsi tersebut, akan dilakukan perkalian matriks `first` dengan `second` dengan hasil disimpan di `result`
```c
// Initializing elements of matrix mult to 0.
for (int i = 0; i < r1; ++i) {
  for (int j = 0; j < c2; ++j) {
      result[i][j] = 0;
  }
}
// Multiplying first and second matrices and storing it in result
for (int i = 0; i < r1; ++i) {
  for (int j = 0; j < c2; ++j) {
      for (int k = 0; k < c1; ++k) {
        result[i][j] += first[i][k] * second[k][j];
      }
  }
}
```
#### Hasil
![soal2a_1](./screenshots/2a_1.png)

### Pengiriman Nilai
1. Pada pengiriman nilai, kita gunakan share memory. Melakukan inisiasi `key`, `shmid`, dan variabel `result` sebagai variabel yang akan dikirim nilainya
```c
key_t key = 1234;

int shmid = shmget(key, sizeof(int[10][10]), IPC_CREAT | 0666);
result = shmat(shmid, NULL, 0);
```
2. Dilakukan `sleep(25)` untuk memberi waktu pemanggilan dari client proses.
3. Bila sudah selesai, dilakukan penutupan share memory
```c
shmdt(result);
shmctl(shmid, IPC_RMID, NULL);
```

## Soal 2b
Pada tahap ini akan melakukan penerimaan nilai dan perhitungan matriks berdasarkan peraturan yang ada
1. Melakukan penerimaan nilai variabel result dari server proses
```c
key_t key = 1234;
int shmid = shmget(key, sizeof(int[10][10]), IPC_CREAT | 0666);
result = shmat(shmid, NULL, 0);
```
![soal2b_1](./screenshots/2b_1.png)

Pada tahap selanjutnya adalah melakukan perhitungan nilai permutasi antar nilai dari matriks `result` pada matriks baru `base` pada indeks/cell yang sama. Perhitungan pada tiap cell harus menggunakan 1 thread sendiri.
2. Melakukan pemasukan nilai matriks `base`
```c
printf("Input matrix B 4x6\n");
for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 6; j++) {
        scanf("%d", &base[i][j]);
    }
}
```
3. Melakukan inisiasi tid dengan size sama dengan jumlah cell
```c
pthread_t tid[4][6];
```
4. Memanggil thread untuk perhitungan tiap cell nya
```c
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
          printf("\n can't create thread : [%s]",strerror(err));
      }
      else
      {
          printf("\n create thread success\n");
      }
  }
}
```
5. Function `createArgs` adalah fungsi untuk menghasilkan variabel bertipe data struct `Args`. Tipe data ini memiliki atribut `newVal` bertipe `long long` untuk menyimpan nilai hasil permutasi, lalu ada 2 atribut `matA` dan `matB` untuk menyimpan nilai cell kedua matriks
```c
typedef struct {
    long long *newVal;
    int matA;
    int matB;
}Args;
```
```c
Args *createArgs(long long *nv, int a, int b) {
    Args *arg = malloc(sizeof(Args));

    arg->newVal = nv;
    arg->matA = a;
    arg->matB = b;

    return arg;
}
```
6. Pada function `process`, kita memproses perhitungan matriks, yaitu menghitung permutasinya. Function `faktorial` adalah sebuah fungsi untuk menghitung nilai faktorial dari argument. \
```c
void* process(void *argument) {
  Args *arg = (Args *)argument;

  if(arg->matA == 0 | arg->matB == 0) *(arg->newVal) = 0;
  else if(arg->matA < arg->matB) *(arg->newVal) = faktorial(arg->matA);
  else *(arg->newVal) = (faktorial(arg->matA) / faktorial(arg->matA - arg->matB));
}
```
7. Lalu terakhir melakukan join thread dengan `pthread_join` untuk tiap cell, pencetaakn matriks hasil perhitungna, serta penutupan share memory

```c
printf("-=- Matrix C -=-\n");
for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 6; j++) {
        pthread_join(tid[i][j], NULL);
    }
}    

display_lld(new, 4, 6);

shmdt(result);
shmctl(shmid, IPC_RMID, NULL);
```

#### Hasil
![soal2b_2](./screenshots/2b_2.png)
![soal2b_3](./screenshots/2b_3.png)

## Soal 2c
Pada soal ini kita diminta untuk mengecek 5 proses teratas dengan command `ps aux | sort -nrk 3,3 | head -5` pada program C
1. Karena terdapat 3 command, maka kita lakukan pemanggilan 2 pipe untuk komunikasi squential
```c
if (pipe(fp1) == -1) {
  fprintf(stderr, "Pipe Failed");
  return 1;
}

if (pipe(fp2) == -1) {
  fprintf(stderr, "Pipe Failed");
  return 1;
}
```
2. Mengeksekusi command pertama dengan cara memanggil child process serta menutup pipe `fp1[0]` dan menduplikasi output ke pipe `fp1[1]`. Selanjutnya mengeksekusi command.
```c
if (child_id == 0) {
  // nutup read
  close(fp1[0]);

  // duplicate write ke terminal
  dup2(fp1[1], STDOUT_FILENO);

  // exec ls
  char *argv[] = {"ps", "aux", NULL};
  execv("/bin/ps", argv);
}
```
3. Selanjutnya pada parent process akan dilakukan pengeksekusian command lainnya dengan pendekatan yang sama pada nomor 2. Namun sebelum itu menutup dulu pipe `fp[1]`
```c
// Close write pipe1
close(fp1[1]);
```
#### Kendala
Awalnya kami tidak berhasil mengeksekusi program ini, terutama pada command terakhir `sort nrk 3,3`. Namun setelah menutup pipe `fp1[1]` tersebut, kami berhasil mengeksekusi program sesuai yang diminta
![soal2c_1](./screenshots/2c_1.png)

Memanggil child process. Pada child proses melakukan menutup pipe write `fp1[1]` agar selesai menulis serta menduplikasi input ke pipe `fp1[0]`, dan menutup pipe read `fp2[0]` serta menduplikasi output pipe write `fp2[1]`. Lalu diakhiri dengan pengeksekusian `sort nrk 3,3`.

Selanjutnya pada parent process, sama halnya dengan sebelumnya, yaitu menutup dahulu pipe write `fp2[1]` serta menduplikasi output ke `fp2[0]`.

```c
child_id = fork();
if (child_id < 0) {
  exit(EXIT_FAILURE);
}
if (child_id == 0) {
  // close write pipe1
  close(fp1[1]);

  // duplicate read
  dup2(fp1[0], STDIN_FILENO);

  // close read
  close(fp2[0]);

  // duplicate write ke termin
  dup2(fp2[1], STDOUT_FILENO);

  // exec
  char *argv[] = {"sort", "-nrk", "3,3", NULL};
  execv("/usr/bin/sort", argv);
} else {
  while ((wait(&status)) > 0)
    ;
  // close write
  close(fp2[1]);

  // dup read ke stdin

  dup2(fp2[0], STDIN_FILENO);

  char *argv[] = {"head", "-5", NULL};
  execv("/usr/bin/head", argv);
}
```
![soal2c_2](./screenshots/2c_2.png)
---
# Soal 3
Pada soal ini ada 3 command yang dapat dijalankan. yaitu `-f`,`-d`, dan `x`. Dalam soal ini menggunakan beberapa fungsi yaitu  `moveFile` , `moveFileUtil` , `cleanFolderFromPath` , `getExt` ,  `lisrecAlphaNum`.

## Command -f
Pada command `-f` yaitu bertujuan untuk membuat kategori pada file yang dipilih setelah command tersebut dengan memasukkan file tersebut ke folder baru yang dikelompokkan berdasarkan extensi seperti : `zip`,`txt`, dan lain-lain. Pada saat proses berlangsung, file yang dilakukan pengkategorian dilakukan secara paralel menggunakan thread sehingga setiap file akan berjalan bersamaan
```
while (argv[i] != NULL) {
      err = pthread_create(&(tid[i - 2]), NULL, &moveFile, (void *)argv[i]);
      if (err != 0)
        printf("File %d: Sad, gagal :(\n", i - 1);
      else
        printf("File %d : Berhasil Dikategorikan\n", i - 1);
      i++;
    }
```
Jika berhasil dalam proses tersebut, pesan yang akan keluar berupa `Berhasil Dikategorikan`. Jika file tersebut gagal dalam prosesnya maka pesan berupa `Sad, gagal :(`. Dalam prosesnya kita dapat melakukan pengkategorian lebih dari satu file pada command ini.
Sebagai contoh: 
```
./soal3 -f soal3/soal3kkk/ahsduas-ahsduas.zip soal3/soal3kkk/bs.TXT
```
Pada command ini kita melakukan pengkategorian pada 2 file. dalam proses pengkategorian ada beberapa fungsi yang digunakan. untuk mendapat extensi dari file tersebut kita menggunakan fungsi `getExt` dan `cleanFolderFromPath`
```
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
```
Pada fungsi diatas kita mengambil filename saja dengan menggunakan command `strchr(str, '/')` sehingga yang terdapat pada format path dengan hanya mengambil kalimat setelah `/` yang terakhir 
```
while (pch != NULL) {
    result = pch + 1;
    pch = strchr(pch + 1, '/');
  }
```
kita mendapat filenamenya beserta extensinya. Selanjutnya menggunakan fungsi `getExt` 
```

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
```
pada fungsi ini kita akan mengeliminasi yang bukan extensi nya dengan menggunakan command `strchr(pch, '.')` tetapi pada fungsi ini kita mengambil `.` yang pertama untuk diambil katanya sehingga tidak perlu dilakukan looping seperti fungsi `cleanFolderFromPath`. Pada command `-f` kita menggunakan fungsi `movefile`.
```
char *fileName = (char *)arg;
  char fileAsli[1000], fileCopy[1000];
  strcpy(fileAsli, fileName);
  strcpy(fileCopy, fileName);

  char *ext = getExt(fileName);
  char *cleanName = cleanFolderFromPath(fileCopy);
```
Pada fungsi ini kita melakukan store extensi file tersebut menggunakan fungsi `getExt` ke variabel `ext` dan filename dengan fungsi `cleanFolderFromPath` ke variabel `cleanName`.
```
 char folderName[120];
  // printf("🚀 cleanName: %s\n", cleanName);

  if (cleanName[0] == '.') {
    sprintf(folderName, "Hidden");
  } else if (ext == NULL) {
    sprintf(folderName, "Unknown");
  }
```
Jika file tersebut merupakan Hidden file maka folderName berisi Hidden dengan melakukan check `if (cleanName[0] == '.')` pada awalan dari file name tersebut. Sedangkan apabila file tersebut tidak memiliki extensi maka akan folderName akan berisi Unknown. Dan untuk yang memiliki extensi akan dijadikan huruf kecil semua karena perintah soal tidak case sensitive.
```
else {
    // TODO lowercase file ext (JPG => jpg)
    for (int i = 0; ext[i]; i++) {
      ext[i] = tolower(ext[i]);
    }
    sprintf(folderName, "%s", ext);
  }
```
yaitu dengan menggunakan fungsi `tolower(ext[i])` kita mendapat filename berupa huruf kecil semua dan di store ke `folderName`. lalu folder baru akan dibuat dengan fungsi `mkdir(folderName, 0777);`. untuk melakukan move file tersebut ke folder yang baru maka dilakukan
```
char destDir[200];
  sprintf(destDir, "%s/%s/%s", curDir, folderName,cleanFolderFromPath(fileAsli));
  moveFileUtil(fileAsli, destDir);
  return NULL;
```
pada awal akan dibuat file difolder baru tersebut yang formatnya terdapat pada variabel `destDir` yang berupa gabungan `curDir, folderName,cleanFolderFromPath(fileAsli)` dan setelah itu dilakukan penyalinan dengan fungsi `(fileAsli, destDir)`.
```
FILE *fp1, *fp2;

  fp1 = fopen(source, "r");
  fp2 = fopen(dest, "w");
```
pada fungsi ini kita melakukan read pada source yang berisi `fileAsli` dan melakukan write pada file `destDir` yang baru. setelah proses selesai maka file source akan dihapus dengan command `remove(source);`. Hasil dari command `-f` dapat dilihat pada screenshot berikut:

### Hasil
![soal3_-f_1](./screenshots/3_-f_1.png)

![soal3_-f_2](./screenshots/3_-f_2.png)

## Command -d
Pada command ini yaitu melakukan pengkategorian pada semua file yang kita pilih foldernya disaat melakukan command. Pada command ini kita hanya bisa melakukan proses tersebut pada satu folder saja. Folder hasil pengkategorian akan berada pada folder soal3 sama seperti command lain. sebagai contoh: `./soal3 -d soal3/soal3kkk`
pada command ini kita menggunakan command `if (strcmp(argv[1], "-d") == 0) {lisrecAlphaNum(argv[2]);}`. sehingga kita menggunakan fungsi `lisrecAlphaNum`.
```
void lisrecAlphaNum(char *basePath) {
  char path[1000], srcPathForThread[1000];
  struct dirent **namelist;
  int n;
  int i = 0;
  n = scandir(basePath, &namelist, NULL, alphasort);
```
pada fungsi ini kita melakukan scan berapa banyak file yang terdapat pada folder terpilih. dengan command `n = scandir(basePath, &namelist, NULL, alphasort)`. alphasort disini bertujuan untuk melakukan sort berdasarkan aplhabeth sehingga dapat menghindari kegagalan disaat ada namafile yang sama dengan extensi dari file lain yaitu dengan melakukan proses pada file tersebut dan masuk ke folder `Unknown` sehingga apabila ada file yang memiliki extensi yang sama dengan file sebelumnya, folder extensi dapat terbuat.
```
if (n < 0)
    return;
  else {
    while (i < n) {
      if (strcmp(namelist[i]->d_name, ".") != 0 &&
          strcmp(namelist[i]->d_name, "..") != 0) {
        strcpy(path, basePath);
        strcat(path, "/");
        strcat(path, namelist[i]->d_name);
```
pada proses ini kita akan mengeliminasi `. ..` yang terdapat pada saat melakukan scan. dan pada variabel path kita mengisi dengan basepath(folder yang kita gunakan pada saat command `-d`) dan menambah `/` serta filename yang sebelumnya kita store di `struct dirent **namelist`. kemudian untuk variabel `srcPathForThread` kita isi hanya dengan `/` dan filename pada namelist.
```
f (namelist[i]->d_type != DT_DIR) {
          int err;
          err = pthread_create(&(tid[threadCount - 2]), NULL, &moveFile,
                               (void *)path);
```
pada saat melakukan scan kita tidak membedakan antara direktori atau file sehingga kita melakukan if jika bukan direktori/folder maka kita lakukan command `err = pthread_create(&(tid[threadCount - 2]), NULL, &moveFile,(void *)path)`yang dimana prosesnya sama pada `command -f`.
```
 if (err != 0)
            printf("File %d: Sad, gagal :(\n", threadCount - 1);
          else
            printf("File %d : Berhasil Dikategorikan\n", threadCount - 1);
          threadCount++;

          for (int p = 0; p < (threadCount - 1); p++)
            pthread_join(tid[p], NULL);
```
jika file tersebut berhasil maka pesan berupa `File %d : Berhasil Dikategorikan` ,`%d` disini merupakan urutan sorting file saat proses berlangsung. dan jika gagal, pesan berupa `File %d: Sad, gagal :(`. untuk setiap file akan diberi satu thread sehingga pada saat proses berlangsung tidak perlu menunggu file sebelumnya selesai diproses.
```
 lisrecAlphaNum(path);
```
jika file yang kita scan merupakan direktori maka kita akan menjadikan variabel `path` tersebut menjadi `basepath` sehingga kita dapat melakukan rekursi pada subdirektori yang terdapat pada folder yang kita pilih. dan jika file tersebut merupakan file berekstensi maka akan tetap masuk ke fungsi `lisrecAlphaNum(path)` tetapi lansung dicegah saat
```
 if (n < 0)
    return;
```
karena pada saat scan akan gagal sehingga n<0.
```
}
      free(namelist[i]);
      ++i;
    }
    free(namelist);
  }
```
dan kita mengosongkan `namelist[i]` serta struct `namelist` agar dapat dilakukan proses selanjutnya tanpa ada gangguan. Hasil dari command `./soal3 -d soal3/soal3kkk` dapat dilihat pada screenshot berikut:

### Hasil
![soal3_-f_2](./screenshots/3_-d.png)

## Command *
pada command ini prosesnya hampir identik dengan command `-d` hanya pada saat command basepath nya berupa current directori yang merupakan folder soal3 sehingga semua file yang ada pada soal3 akan dilakukan pengkategorian. Command nya berupa `./soal3 \*` . 
```
if (strcmp(argv[1], "*") == 0) {
    lisrecAlphaNum(".");
```
pada fungsi ini kita mengisi `basepath` pada fungsi `lisrecAlphaNum` berupa curent directori yaitu dengan `"."`. sehingga pada saat fungsi `lisrecAlphaNum` semua file yang terdapat pada folder soal3 akan dikategorikan semuanya. Hasil dari command tersebut dapat dilihat sebagai berikut:

### Hasil
![soal3_-f_2](./screenshots/3_*.png)
 
Dari hasil tersebut terlihat bahwa pada folder soal3 semua file nya berhasil dikategorikan sehingga semua file masuk ke folder berdasarkan kategori masing masing

### Kendala
1. untuk melakukan kategori file yang ekstensinya sama dengan dengan filename tidak, proses pengkategorian gagal dikarenakan folder tidak dapat dibuat dengan nama yang sama dengan file yang ada di folder tersebut. sehingga dibutuhkan metode aplhabeth sorting

## Referensi

### 1

- https://stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c

### 2
- https://stackoverflow.com/questions/32284887/stuck-on-using-multiple-pipes-in-c

### 3
