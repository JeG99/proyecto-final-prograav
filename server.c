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

char *genome;

char *strremove(char *str, const char *sub) {
  char *p, *q, *r;
  if ((q = r = strstr(str, sub)) != NULL) {
      size_t len = strlen(sub);
      while ((r = strstr(p = r + len, sub)) != NULL) {
          while (p < r)
              *q++ = *p++;
      }
      while ((*q++ = *p++) != '\0')
          continue;
  }
  return str;
}

void read_genome(int client_socket) {
  char *buff;
  buff = malloc(MAX_LEN);
  genome = realloc(genome, MAX_LEN);

  int first_pass = 1;
  int end_flag = 0;

  for (;;) {
    bzero(buff, MAX_LEN);
    recv(client_socket , buff , MAX_LEN , 0);
    buff[strcspn(buff, "\r\n")] = 0;

    if (first_pass == 0) {
      genome = realloc(genome, strlen(genome) + MAX_LEN + 1);
    }

    first_pass = 0;

    if (strstr(buff, "END") != NULL){
      buff = strremove(buff, "END");
      end_flag = 1;
    }

    strcat(genome, buff);

    if (end_flag) break;
  }

  printf("%lu\n", strlen(genome));

  return;
}

void search_sequences(int client_socket) {
  char *buff;
  buff = malloc(MAX_LEN);

  int number_of_test = 1;
  int end_flag = 0;

  for (;;) {
    bzero(buff, MAX_LEN);
    recv(client_socket , buff , MAX_LEN , 0);
    buff[strcspn(buff, "\r\n")] = 0;

    if (strstr(buff, "END") != NULL){
      buff = strremove(buff, "END");
      end_flag = 1;
    }

    printf("Test #%d = ", number_of_test);

    if (strstr(genome, buff) == NULL) {
      printf("Not found\n");
    } else {
      printf("Found \n");
    }
    number_of_test++;

    if(end_flag) break;
  }
}

int main () {
  // create socket
  int server_socket;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  // address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(9002);
  server_address.sin_addr.s_addr = INADDR_ANY;

  // bind
  bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
  
  // listen
  listen(server_socket, 3);

  // accept
  int client_socket;
  client_socket = accept(server_socket, NULL, NULL);

/* ---------------------------- Socket Connection --------------------------- */
  int read_size;
  char msg[MAX_LEN];

  do {
    bzero(msg, MAX_LEN);
    read_size = recv(client_socket , msg , MAX_LEN , 0);
    
    if (read_size > 0) {
      printf("El servidor recibio el comando = %s\n\n", msg);

      if (strcmp(msg, "1") == 0) {
        read_genome(client_socket);
      }

      if (strcmp(msg, "2") == 0) {
        search_sequences(client_socket);
      }
      
    }
  } while (read_size > 0);

  if (read_size == 0) {
    puts("Client disconnected");
  } else if(read_size == -1) {
    perror("recv failed");
  }

  close(server_socket);

  return 0;
}
