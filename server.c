#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

int get_digits(int n) {
  int count = 0;  
  while(n!=0) {  
    n=n/10;  
    count++;  
  }
  return count;
}

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
  char msg[MAX_LEN];
  buff = malloc(MAX_LEN);
  free(genome);
  genome = NULL;
  genome = malloc(MAX_LEN);

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

  sprintf(msg, "%d", (int) strlen(genome));
  //printf("%d\n", (int) strlen(msg));
  for(;;) {
    if(send(client_socket, msg, strlen(msg), 0) < 0) {
      puts("Send failed");
      break;
    } else {
      break;
    }
  }
  //printf("%lu\n", strlen(genome));
  printf("%s\n", genome);
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
  int start = (int)arg;
  for (int index = start; index < start + sequences.pool_size && index < sequences.size; index++) {
    char* search = strstr(genome, sequences.seqs[index]);
    // printf("Test #%d = ", index);
    if (search == NULL) {
      // printf("Not found\n");
    } else {
      int idx = search - genome;
      // printf("Found at index %d\n", idx);
      sequences.found_idx[index] = idx;
    }
  }

}



void search_sequences(int client_socket) {
  if(sequences.size > 0) {
    for(int i = 0; i < sequences.size; i++) {
      free(sequences.seqs[i]);
    }
    free(sequences.found_idx);
    sequences.size = 0;
    sequences.pool_size = 8;
  }
  //if(sequences.seqs[0] != NULL) printf("%s\n", sequences.seqs[0]);
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
    sequences.found_idx[sequences.size - 1] = -1;
    if(end_flag) break;
  }
  printf("%d\n", sequences.size);
  for(int i = 0; i < sequences.size; i++) {
    printf("%s\n", sequences.seqs[i]);
  }

  // Search by threads
  int rc, idx, max_threads;

  int thread_no = 8;

  sequences.pool_size = ceil((double)sequences.size / thread_no);

  // if(sequences.size < sequences.pool_size) {
  //   max_threads = sequences.size;
  // } else {
  //   max_threads = ceil((double)sequences.size / sequences.pool_size);
  // }
  // if (max_threads == 0) max_threads = 1;

  max_threads = thread_no;

  pthread_t threads[max_threads];
  for (int i = 0, k = 0; i < max_threads; i++, k += sequences.pool_size) {
      rc = pthread_create(&threads[i], NULL, map, (void *)(k));
  }

  for (int i = 0; i < max_threads; i++) {
      rc = pthread_join(threads[i], NULL);
  }


}

void make_swap(int i, int j) {
  int tmp_idx = sequences.found_idx[i];
  sequences.found_idx[i] = sequences.found_idx[j];
  sequences.found_idx[j] = tmp_idx;

  char* tmp_seq = sequences.seqs[i];
  sequences.seqs[i] = sequences.seqs[j];
  sequences.seqs[j] = tmp_seq;
}


int sort_partition(int low, int high) {
  int pivot = sequences.found_idx[high];
  
  int i = (low - 1);
  for (int j = low; j < high; j++) {
    if (sequences.found_idx[j] <= pivot) {
      i++;
      make_swap(i, j);
    }
  }

  make_swap(i + 1, high);
  
  return (i + 1);
}

void sort_sequences(int low, int high) {
  if (low < high) {
    int pi = sort_partition(low, high);
    sort_sequences(low, pi - 1);    
    sort_sequences(pi + 1, high);
  }
}

int main () {

  char send_buff[MAX_LEN];
  memset(send_buff, 0, MAX_LEN);

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

wait_for_new_connection:
	puts("Waiting for incoming connections...");
  
  // accept
  int client_socket;
  client_socket = accept(server_socket, NULL, NULL);
	puts("Client connection accepted");
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
        char int_buff[1024];
        char double_buff[1024];
        char resp[1024]; // = "Seq #";

        search_sequences(client_socket);
        sort_sequences(0, sequences.size - 1);
        int mapped_count = 0;
        double mapped_percent = 0;
        int first = 1;
        int last_mapped = -1;
        for (int i = 0; i < sequences.size; i++) {
          memset(resp, 0, strlen(resp));
          strncat(resp, "Seq #", strlen("Seq #"));
          //printf("Seq #%d = ", i);
          memset(int_buff, 0, strlen(int_buff));
          sprintf(int_buff, "%d", i);
          strncat(resp, int_buff, strlen(int_buff));
          int idx = sequences.found_idx[i];
          if (idx < 0) {
            //printf("Not found\n");
            strncat(resp, " = Not found\n", strlen(" = Not found\n"));
          } else {
            //printf("Found at index %d\n", idx);
            strncat(resp, " = Found at index ", strlen(" = Found at index "));
            memset(int_buff, 0, strlen(int_buff));
            sprintf(int_buff, "%d\n", idx);
            strncat(resp, int_buff, strlen(int_buff));
            if (first) {
              first = 0;
              mapped_percent += (double)strlen(sequences.seqs[i]) / strlen(genome);
              last_mapped = i;
            } else if (!first) {
              int i_first = sequences.found_idx[last_mapped], i_last = sequences.found_idx[last_mapped] + strlen(sequences.seqs[last_mapped]) - 1;
              int j_first = sequences.found_idx[i], j_last = sequences.found_idx[i] + strlen(sequences.seqs[i]) - 1;

              if (j_first > i_last) {
                mapped_percent += (double)strlen(sequences.seqs[i]) / strlen(genome);
                last_mapped = i;
              } else if (j_last - i_last > 0) {
                mapped_percent += (double)(j_last - i_last) / strlen(genome);
                last_mapped = i;
              } 
            }
            mapped_count++;
          }
          strncat(send_buff, resp, strlen(resp));
          //printf("GENERADA: %s\n", resp);

        }

        //printf("%s", send_buff);
        
        memset(double_buff, 0, strlen(double_buff));
        sprintf(double_buff, "El archivo cubre el %.2f %% del genoma de referencia\n", mapped_percent * 100);
        strncat(send_buff, double_buff, strlen(double_buff));

        //printf("El archivo cubre el %.2f %% del genoma de referencia\n", mapped_percent * 100);
        
        memset(int_buff, 0, strlen(int_buff));
        sprintf(int_buff, "%d secuencias mapeadas\n", mapped_count);
        strncat(send_buff, int_buff, strlen(int_buff));

        //printf("%d secuencias mapeadas\n", mapped_count);
        
        memset(int_buff, 0, strlen(int_buff));
        sprintf(int_buff, "%d secuencias no mapeadas\n", sequences.size - mapped_count);
        strncat(send_buff, int_buff, strlen(int_buff));
        
        //printf("%d secuencias no mapeadas\n", sequences.size - mapped_count);

        //printf("%s", send_buff);

        for(;;) {
          if(send(client_socket, send_buff, strlen(send_buff), 0) < 0) {
            puts("Send failed");
            break;
          } else {
            break;
          }
        }
        
      }
      
    }
  } while (read_size > 0);

  // for(int i = 0; i < sequences.size; i++) {
  //   printf("%s\n", sequences.seqs[i]);
  // }

  if (read_size == 0) {
    puts("Client disconnected");
    goto wait_for_new_connection;
  } else if(read_size == -1) {
    perror("recv failed");
  }

  close(server_socket);

  return 0;
}
