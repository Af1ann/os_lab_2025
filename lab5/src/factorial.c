// factorial.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <string.h>

// Глобальные переменные
int k = 0;
int pnum = 1;
int mod = 0;
unsigned long long result = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Структура для передачи данных в поток
typedef struct {
    int start;
    int end;
} ThreadData;

// Функция потока для вычисления части факториала
void* calculate_partial_factorial(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    unsigned long long partial_result = 1;
    
    printf("Thread calculating from %d to %d\n", data->start, data->end);
    
    for (int i = data->start; i <= data->end; i++) {
        partial_result = (partial_result * i);
        printf("  Partial result after %d: %llu\n", i, partial_result);
    }
    
    // Применяем модуль к частичному результату
    partial_result %= mod;
    
    // Защищаем обновление глобального результата мьютексом
    pthread_mutex_lock(&mutex);
    printf("Thread updating result: %llu * %llu mod %d\n", result, partial_result, mod);
    result = (result * partial_result) % mod;
    pthread_mutex_unlock(&mutex);
    
    free(data);
    return NULL;
}

void print_usage(char* program_name) {
    printf("Usage: %s -k <number> --pnum=<threads> --mod=<modulus>\n", program_name);
    printf("Example: %s -k 10 --pnum=4 --mod=1000\n", program_name);
}

int main(int argc, char** argv) {
    // Парсинг аргументов командной строки
    static struct option options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    // Парсим аргументы
    while ((c = getopt_long(argc, argv, "k:p:m:", options, &option_index)) != -1) {
        switch (c) {
            case 'k':
                k = atoi(optarg);
                break;
            case 'p':
                pnum = atoi(optarg);
                break;
            case 'm':
                mod = atoi(optarg);
                break;
            case '?':
                print_usage(argv[0]);
                return 1;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // Проверка входных данных
    if (k <= 0 || pnum <= 0 || mod <= 0) {
        printf("Error: All parameters must be positive numbers\n");
        print_usage(argv[0]);
        return 1;
    }
    
    if (pnum > k) {
        printf("Warning: pnum (%d) cannot be greater than k (%d). Setting pnum = %d\n", pnum, k, k);
        pnum = k;
    }
    
    printf("Computing %d! mod %d using %d threads\n", k, mod, pnum);
    
    // Создание потоков
    pthread_t threads[pnum];
    int numbers_per_thread = k / pnum;
    int remainder = k % pnum;
    int current_start = 1;
    
    printf("Distribution: %d numbers per thread, remainder: %d\n", numbers_per_thread, remainder);
    
    for (int i = 0; i < pnum; i++) {
        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        data->start = current_start;
        data->end = current_start + numbers_per_thread - 1;
        
        // Распределяем остаток по первым потокам
        if (remainder > 0) {
            data->end++;
            remainder--;
        }
        
        printf("Thread %d: numbers from %d to %d\n", i, data->start, data->end);
        current_start = data->end + 1;
        
        if (pthread_create(&threads[i], NULL, calculate_partial_factorial, data) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Ожидание завершения всех потоков
    for (int i = 0; i < pnum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }
    
    printf("\nFinal result: %llu\n", result);
    
    // Уничтожение мьютекса
    pthread_mutex_destroy(&mutex);
    
    return 0;
}