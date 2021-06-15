#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Socket imports
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Multithreading
#include <pthread.h>

#define MAX_LEN 1000000

char *genome;

struct SequencesStruct {
  int pool_size;
  int size;
  char **seqs;
  int *found_idx;
} sequences;

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

char **append(char **oldMatrix, int *size, const char str[MAX_LEN]) {
  for(int i = 0; i < *size; i++){
    if(strcmp(oldMatrix[i], str) == 0){
      return oldMatrix;
    }
  }

  char **newMatrix = (char **)realloc(oldMatrix, (*size + 1)*sizeof(char *));
  newMatrix[*size] = (char *)malloc(MAX_LEN*sizeof(char));
  strcpy(newMatrix[*size], str);
  (*size)++;

  return newMatrix;
}

void *map(void* arg) {

  int max_threads;

  if(sequences.size < sequences.pool_size) {
    max_threads = sequences.size;
  } else {
    max_threads = sequences.pool_size;
  }
  int start = (int)arg;
  for (int index = start; index < start + max_threads && index < sequences.size; index++) {
    printf("Test #%d %s = ", index, sequences.seqs[index]);
    char* search = strstr(genome, sequences.seqs[index]);
    if (search == NULL) {
      printf("Not found\n");
    } else {
      int idx = search - genome;
      printf("Found at index %d\n", idx);
      sequences.found_idx[index] = idx;
    }
  }

}

void search_sequences(int client_socket) {
  char *buff;
  buff = malloc(MAX_LEN);

  int number_of_test = 0;
  int end_flag = 0;

  for (;;) {
    bzero(buff, MAX_LEN);
    recv(client_socket , buff , MAX_LEN , 0);
    buff[strcspn(buff, "\r\n")] = 0;

    if (strstr(buff, "END") != NULL){
      buff = strremove(buff, "END");
      end_flag = 1;
    }
    sequences.seqs = append(sequences.seqs, &sequences.size, buff);
    sequences.found_idx = (int *)realloc(sequences.found_idx, (sequences.size) * sizeof(int));
    sequences.found_idx[sequences.size] = -1;
    if(end_flag) break;
  }

  // Search by threads
  int rc, idx, max_threads;

  if(sequences.size < sequences.pool_size) {
    max_threads = sequences.size;
  } else {
    max_threads = sequences.pool_size;
  }
  if (max_threads == 0) max_threads = 1;

  pthread_t threads[max_threads];
  for (int i = 0, k = 0; i < sequences.size / max_threads; i++, k += max_threads) {
      rc = pthread_create(&threads[i], NULL, map, (void *)(k));
  }

  for (int i = 0; i < sequences.size; i+=max_threads) {
      rc = pthread_join(threads[i], NULL);
  }


}

int main () {

  sequences.size = 0;
  sequences.pool_size = 8;

  // create socket
  int server_socket;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  // address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(9002);
  server_address.sin_addr.s_addr = INADDR_ANY;

  // bind
  if(bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    //print the error message
    perror("bind failed. Error");
    return 1;
  }
  puts("bind done");
  
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

  for(int i = 0; i < sequences.size; i++) {
    printf("%s\n", sequences.seqs[i]);
  }

  if (read_size == 0) {
    puts("Client disconnected");
  } else if(read_size == -1) {
    perror("recv failed");
  }

  close(server_socket);

  return 0;
}
