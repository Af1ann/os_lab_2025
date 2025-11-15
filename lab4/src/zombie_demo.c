/* zombie_demo.c - программа для демонстрации зомби процессов */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    
    printf("Родительский процесс PID: %d\n", getpid());
    
    pid = fork();
    
    if (pid < 0) {
        perror("Ошибка fork");
        exit(1);
    }
    else if (pid == 0) {
        /* Дочерний процесс */
        printf("Дочерний процесс PID: %d завершается\n", getpid());
        exit(0);
    }
    else {
        /* Родительский процесс */
        printf("Родитель создал дочерний процесс PID: %d\n", pid);
        printf("Родитель засыпает на 30 секунд (дочерний станет зомби)\n");
        printf("Используйте команду: ps aux | grep zombie_demo\n");
        printf("или: ps aux | grep Z\n");
        printf("Вы увидите процесс со статусом Z (zombie)\n\n");
        
        sleep(30);
        
        printf("\nРодитель вызывает wait() для очистки зомби\n");
        wait(NULL);
        
        printf("Зомби процесс очищен. Родитель завершается.\n");
        sleep(5);
    }
    
    return 0;
}
