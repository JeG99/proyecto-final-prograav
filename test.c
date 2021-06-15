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
  char* strRes;
  str = malloc(MAX_LEN);

  FILE *fp;

  fp = fopen("test1.txt", "r");
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

  strRes = malloc(strlen(str) + 1);
  strcpy(strRes, str);

  buff = realloc(buff, MAX_LEN);
  
  fclose(fp);

  fp = fopen("seq1.txt", "r");
  if (fp == NULL) {
    perror("Error al abrir el archivo");
  }

  int number_of_test = 1;

  while (fgets(buff, MAX_LEN, fp) != NULL) {
    buff[strcspn(buff, "\r\n")] = 0;

    printf("Test #%d = ", number_of_test);

    char *p = strstr(str, buff);

    if (p == NULL) {
      printf("Not found\n");
    } else {
      int start = p-str;
      // int end = start + strlen(p);

      printf("Found, starting in %d\n", start);

      for (int i = 0; i < strlen(buff); i++) {
        strRes[start + i] = '1';
      }
    }

    number_of_test++;
  }

  printf("\nResult genome\n%s", strRes);

  free(buff);
  free(str);

  return 0;
}
