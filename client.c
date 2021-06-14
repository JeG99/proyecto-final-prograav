#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Socket imports
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_LEN 1000000

void upload_genome(char* fName, int server_socket) {
  FILE *fp;

  fp = fopen(fName, "r");
  if (fp == NULL) {
    perror("Error al abrir el archivo");
  }

  char *buff;
  buff = malloc(MAX_LEN);

  while (fgets(buff, MAX_LEN, fp) != NULL) {
    send(server_socket, buff, strlen(buff), 0);
  }
  
  fclose(fp);
  
  char* endMsg = "END_GENOME";
  send(server_socket, endMsg, strlen(endMsg), 0);

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
    printf("Lista de comandos:\n\n1) Subir genoma\n2) Salir\n\nIngrese el numero de su opcion: ");
    scanf("%d", &command);

    if (command == 1) {
      char fName[100];
      printf("\n======== SUBIR GENOMA ========\nIngrese el nombre del archivo: ");
      scanf("%s", fName);
      strcpy(msg, "1");
      send(server_socket, msg, strlen(msg), 0);
      upload_genome(fName, server_socket);
    }

    printf("\n");
  } while(command != 2);

  close(server_socket);

  return 0;
}
