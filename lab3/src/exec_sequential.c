#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s seed arraysize\n", argv[0]);
        return 1;
    }

    int seed = atoi(argv[1]);
    int array_size = atoi(argv[2]);

    if (seed <= 0 || array_size <= 0) {
        printf("Both seed and array_size must be positive numbers\n");
        return 1;
    }

    pid_t child_pid = fork();
    
    if (child_pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (child_pid == 0) {
        // Дочерний процесс
        char seed_str[20], array_size_str[20];
        
        // Преобразуем числа в строки
        sprintf(seed_str, "%d", seed);
        sprintf(array_size_str, "%d", array_size);
        
        // Аргументы для exec
        char *args[] = {"./sequential_min_max", seed_str, array_size_str, NULL};
        
        // Запускаем программу
        execvp(args[0], args);
        
        // Если execvp вернул управление, значит произошла ошибка
        perror("execvp failed");
        exit(1);
    } else {
        // Родительский процесс
        int status;
        waitpid(child_pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Child process exited with status: %d\n", WEXITSTATUS(status));
        } else {
            printf("Child process terminated abnormally\n");
        }
    }

    return 0;
}