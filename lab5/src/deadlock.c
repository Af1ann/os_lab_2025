// deadlock.c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

// Два мьютекса для демонстрации deadlock
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_function(void* arg) {
    printf("Thread 1: Starting...\n");
    
    printf("Thread 1: Trying to lock mutex1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 1: Successfully locked mutex1\n");
    
    // Имитация работы с ресурсом
    printf("Thread 1: Working with resource protected by mutex1...\n");
    sleep(2);
    
    printf("Thread 1: Trying to lock mutex2...\n");
    pthread_mutex_lock(&mutex2); // DEADLOCK! Ждет mutex2, который удерживается Thread 2
    printf("Thread 1: Successfully locked mutex2\n");
    
    // Критическая секция (никогда не выполнится из-за deadlock)
    printf("Thread 1: Entering critical section (should never see this!)\n");
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    printf("Thread 1: Finished (should never see this!)\n");
    return NULL;
}

void* thread2_function(void* arg) {
    printf("Thread 2: Starting...\n");
    
    printf("Thread 2: Trying to lock mutex2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Thread 2: Successfully locked mutex2\n");
    
    // Имитация работы с ресурсом
    printf("Thread 2: Working with resource protected by mutex2...\n");
    sleep(2);
    
    printf("Thread 2: Trying to lock mutex1...\n");
    pthread_mutex_lock(&mutex1); // DEADLOCK! Ждет mutex1, который удерживается Thread 1
    printf("Thread 2: Successfully locked mutex1\n");
    
    // Критическая секция (никогда не выполнится из-за deadlock)
    printf("Thread 2: Entering critical section (should never see this!)\n");
    
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    
    printf("Thread 2: Finished (should never see this!)\n");
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    
    printf("=== DEADLOCK DEMONSTRATION ===\n");
    printf("This program will create two threads that cause deadlock.\n");
    printf("The program will hang and need to be terminated with Ctrl+C\n\n");
    
    // Создаем потоки
    if (pthread_create(&thread1, NULL, thread1_function, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    
    if (pthread_create(&thread2, NULL, thread2_function, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    
    // Даем потокам время для выполнения (до deadlock)
    sleep(1);
    printf("\nMain: Threads created and running...\n");
    printf("Main: Waiting for threads to finish (they won't because of deadlock)...\n");
    
    // Пытаемся ждать завершения потоков (бесконечно)
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("Main: All threads finished (should never see this!)\n");
    
    return 0;
}