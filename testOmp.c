#include <stdio.h>
#include <omp.h>

int main () {
  omp_set_num_threads(4);

  int i;

  #pragma omp parallel
  {
    #pragma omp for
    for (i = 0; i < 10; i++) {
      printf("Iter %d from thread %d\n", i, omp_get_thread_num());
    }
  }

  return 0;
}
