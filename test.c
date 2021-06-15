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

int main () {
  char* str;
  str = malloc(MAX_LEN);

  FILE *fp;

  fp = fopen("long_test.txt", "r");
  if (fp == NULL) {
    perror("Error al abrir el archivo");
  }

  char *buff;
  buff = malloc(MAX_LEN);

  int first_pass = 1;

  while (fgets(buff, MAX_LEN, fp) != NULL) {
    if (first_pass == 0) {
      str = realloc(str, strlen(str) + MAX_LEN + 1);
    }
    strcat(str, buff);
    first_pass = 0;
  }

  buff = realloc(buff, MAX_LEN);
  
  fclose(fp);

  fp = fopen("long_seq.txt", "r");
  if (fp == NULL) {
    perror("Error al abrir el archivo");
  }

  int number_of_test = 1;

  while (fgets(buff, MAX_LEN, fp) != NULL) {
    buff[strcspn(buff, "\r\n")] = 0;

    printf("Test #%d = ", number_of_test);
    if (strstr(str, buff) == NULL) {
      printf("Not found\n");
    } else {
      printf("Found \n");
    }
    number_of_test++;
  }

  free(buff);
  free(str);

  return 0;
}
