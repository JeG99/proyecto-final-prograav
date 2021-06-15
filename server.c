#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h> 
#include <omp.h>

// Socket imports
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_LEN 1000000
#define NUM_THREADS 4 // = # of cores in macbook pro

char *genome;
char *genomeCheck;

typedef struct {
  char **array;
  size_t used;
  size_t size;
} Array;

void initArray(Array *a, size_t initialSize) {
  a->array = malloc(initialSize * sizeof(char*));
  a->used = 0;
  a->size = initialSize;
}

void insertArray(Array *a, char *element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = realloc(a->array, a->size * sizeof(char*));
  }

  a->array[a->used] = malloc(strlen(element) + 1);
  strcpy(a->array[a->used], element);
  a->used++;
}

void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

Array sequences;
Array sequencesResults;

int found_seq;
int total_seq;
int amount_found;

int msleep(long msec) {
  struct timespec ts;
  int res;

  if (msec < 0) {
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

void read_genome(int client_socket) {
  char *buff;
  buff = malloc(MAX_LEN);
  genome = realloc(genome, MAX_LEN);

  int first_pass = 1;

  for (;;) {
    bzero(buff, MAX_LEN);
    recv(client_socket , buff , MAX_LEN , 0);
    buff[strcspn(buff, "\r\n")] = 0;

    if (first_pass == 0) {
      genome = realloc(genome, strlen(genome) + MAX_LEN + 1);
    }

    first_pass = 0;

    if (strcmp(buff, "END") == 0) {
      break;
    }

    strcat(genome, buff);
  }

  genomeCheck = malloc(strlen(genome) + 1);
  strcpy(genomeCheck, genome);

  // printf("%lu\n", strlen(genome));

  return;
}

void read_sequences(int client_socket) {
  char *buff;
  buff = malloc(MAX_LEN);
  initArray(&sequences, 10);

  total_seq = 0;

  for (;;) {
    bzero(buff, MAX_LEN);
    recv(client_socket , buff , MAX_LEN , 0);
    buff[strcspn(buff, "\r\n")] = 0;

    if (strcmp(buff, "END") == 0) {
      break;
    }

    insertArray(&sequences, buff);
    total_seq++;
  }
}

void search_sequences(int client_socket) {
  found_seq = 0;
  amount_found = 0;
  initArray(&sequencesResults, 10);

  omp_set_num_threads(NUM_THREADS);

  #pragma omp parallel shared(found_seq, amount_found, sequencesResults, genomeCheck, genome)
  {
    #pragma omp for
    for (int i = 0; i < sequences.used; i++) {

      // printf("Iter %d from thread %d\n", i, omp_get_thread_num());

      // Check if seq is in genome;
      char *p = strstr(genome, sequences.array[i]);

      // res holds the result string
      char *res;
      res = malloc(MAX_LEN);
      char *appendRes = res;

      appendRes += sprintf(appendRes, "Seq #%d = ", i+1);

      if (p == NULL) {
        sprintf(appendRes, "No se encontro");
        // printf("No se encontro\n");
      } else {
        #pragma omp critical
        {
          found_seq++;
          for (int j = 0; j < strlen(sequences.array[i]); j++) {
            if (genomeCheck[p-genome + j] != '1') {
              genomeCheck[p-genome + j] = '1';
              amount_found++;
            }
          }
        }
        sprintf(appendRes, "Se encontro a partir del caracter %ld\n", p-genome);
        // printf("Se encontro a partir del caracter %ld\n", p-genome);
      }

      #pragma omp critical
      {
        insertArray(&sequencesResults, res);
      }
    }
  }

  char *buff;
  buff = malloc(MAX_LEN);

  // Send results
  for (int i = 0; i < sequencesResults.used; i++) {
    bzero(buff, MAX_LEN);

    strcpy(buff, sequencesResults.array[i]);
    send(client_socket, buff, strlen(buff), 0);
    msleep(25);
  }

  bzero(buff, MAX_LEN);
  strcpy(buff, "END");
  send(client_socket, buff, strlen(buff), 0);
  msleep(25);

  double perc = (double) amount_found / strlen(genome);
  // printf("%d %lu %f\n", amount_found, strlen(genome), perc);
  
  bzero(buff, MAX_LEN);
  sprintf(buff, "El archivo cubre el %c%f del genoma de referencia\n", 37, perc * 100);
  send(client_socket, buff, strlen(buff), 0);
  msleep(25);

  bzero(buff, MAX_LEN);
  sprintf(buff, "%d secuencias mapeadas\n", found_seq);
  send(client_socket, buff, strlen(buff), 0);
  msleep(25);

  bzero(buff, MAX_LEN);
  sprintf(buff, "%d secuencias no mapeadas\n", total_seq - found_seq);
  send(client_socket, buff, strlen(buff), 0);
  msleep(25);
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
        read_sequences(client_socket);
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
