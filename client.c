#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h> 

// Socket imports
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_LEN 1000000

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void upload_genome(char* fName, int server_socket) {
  char resp[MAX_LEN];
  char fname[1024];
  FILE *fp;

  fp = fopen(fName, "r");
  while(fp == NULL) {
    printf("Este archivo no existe, intente de nuevo: ");
    scanf("%s", fname);
    fp = fopen(fname, "r");
  }

  if (fp == NULL) {
    perror("Error al abrir el archivo");
  }

  char *buff;
  buff = malloc(MAX_LEN);

  while (fgets(buff, MAX_LEN, fp) != NULL) {
    buff[strcspn(buff, "\r\n")] = 0;
    send(server_socket, buff, strlen(buff), 0);
  }
  
  fclose(fp);
  
  char* endMsg = "END_GENOME";
  send(server_socket, endMsg, strlen(endMsg), 0);
  memset(resp, 0, MAX_LEN);
  for(;;) {
    if( recv(server_socket , resp , MAX_LEN , 0) < 0) {
      puts("recv failed");
      break;
    } else { 
      break;
    }
  }

  printf("Genome size: %s\n", resp);

  free(buff);

  return;
}

void search_sequences(char* fName, int server_socket) {
  char resp[MAX_LEN];
  char fname[1024];
  FILE *fp;

  fp = fopen(fName, "r");
  while(fp == NULL) {
    printf("Este archivo no existe, intente de nuevo: ");
    scanf("%s", fname);
    fp = fopen(fname, "r");
  }

  if (fp == NULL) {
    perror("Error al abrir el archivo");
  }

  char *buff;
  buff = malloc(MAX_LEN);

  while (fgets(buff, MAX_LEN, fp) != NULL) {
    buff[strcspn(buff, "\r\n")] = 0;
    msleep(10);
    send(server_socket, buff, strlen(buff), 0);
  }
  
  fclose(fp);
  
  char* endMsg = "END";
  send(server_socket, endMsg, strlen(endMsg), 0);

  memset(resp, 0, MAX_LEN);
  for(;;) {
    if( recv(server_socket , resp , MAX_LEN , 0) < 0) {
      puts("recv failed");
      break;
    } else { 
      break;
    }
  }

  printf("%s", resp);

  free(buff);

  return;
}

int main () {
  // create socket
  int server_socket;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  // address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(9002);
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

  // connect
  int connection_status = connect(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
  if (connection_status == -1) {
    printf("Socket error: error al crear la connecion\n\n");
  }

  int command;
  char msg[100];

  do {
    bzero(msg, 100);
    printf("Lista de comandos:\n\n1) Subir genoma\n2) Buscar sequencias\n3) Salir\n\nIngrese el numero de su opcion: ");
    scanf("%d", &command);

    if (command == 1) {
      char fName[100];
      printf("\n======== SUBIR GENOMA ========\nIngrese el nombre del archivo: ");
      scanf("%s", fName);
      strcpy(msg, "1");
      send(server_socket, msg, strlen(msg), 0);
      upload_genome(fName, server_socket);
    }

    if (command == 2) {
      char fName[100];
      printf("\n======== BUSCAR SEQUENCIAS ========\nIngrese el nombre del archivo: ");
      scanf("%s", fName);
      strcpy(msg, "2");
      send(server_socket, msg, strlen(msg), 0);
      search_sequences(fName, server_socket);
    }

    if (command != 1 && command != 2 && command != 3) {
      printf("--------Este comando no existe, por favor intente de nuevo--------\n");
    }

    printf("\n");
  } while(command != 3);

  close(server_socket);

  return 0;
}
