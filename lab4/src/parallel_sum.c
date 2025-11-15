/* parallel_sum.c - параллельное вычисление суммы массива */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/time.h>

struct SumArgs {
  int *array;
  int begin;
  int end;
};

void GenerateArray(int *array, unsigned int array_size, unsigned int seed) {
  srand(seed);
  for (unsigned int i = 0; i < array_size; i++) {
    array[i] = rand() % 100;
  }
}

int Sum(const struct SumArgs *args) {
  int sum = 0;
  for (int i = args->begin; i < args->end; i++) {
    sum += args->array[i];
  }
  return sum;
}

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  struct option long_options[] = {
    {"threads_num", required_argument, NULL, 't'},
    {"array_size", required_argument, NULL, 'a'},
    {"seed", required_argument, NULL, 's'},
    {NULL, 0, NULL, 0}
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
    switch (opt) {
      case 't':
        threads_num = atoi(optarg);
        break;
      case 'a':
        array_size = atoi(optarg);
        break;
      case 's':
        seed = atoi(optarg);
        break;
      default:
        fprintf(stderr, "Usage: %s --threads_num <num> --array_size <num> --seed <num>\n", 
                argv[0]);
        return 1;
    }
  }

  if (threads_num == 0 || array_size == 0) {
    fprintf(stderr, "Error: threads_num and array_size must be greater than 0\n");
    return 1;
  }

  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  int *array = malloc(sizeof(int) * array_size);
  struct SumArgs *args = malloc(sizeof(struct SumArgs) * threads_num);

  GenerateArray(array, array_size, seed);

  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);

  uint32_t chunk_size = array_size / threads_num;
  uint32_t remainder = array_size % threads_num;

  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * chunk_size;
    args[i].end = (i + 1) * chunk_size;

    if (i == threads_num - 1) {
      args[i].end += remainder;
    }

    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int sum = 0;
    pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }

  gettimeofday(&end_time, NULL);
  long elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000000L +
                      (end_time.tv_usec - start_time.tv_usec);

  free(args);
  free(array);
  free(threads);

  printf("Total: %d\n", total_sum);
  printf("Elapsed time: %ld us (%.6f seconds)\n", 
         elapsed_time, elapsed_time / 1000000.0);

  return 0;
}
