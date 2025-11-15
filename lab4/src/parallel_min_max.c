/* parallel_min_max.c - поиск min/max с поддержкой таймаута */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <getopt.h>
#include <sys/time.h>
#include <string.h>

struct MinMax {
    int min;
    int max;
};

volatile sig_atomic_t timeout_occurred = 0;

void alarm_handler(int signum) {
    (void)signum;
    timeout_occurred = 1;
}

void GenerateArray(int *array, unsigned int array_size, unsigned int seed) {
    srand(seed);
    for (unsigned int i = 0; i < array_size; i++) {
        array[i] = rand() % 10000 - 5000;
    }
}

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
    struct MinMax result;
    result.min = array[begin];
    result.max = array[begin];
    
    for (unsigned int i = begin; i < end; i++) {
        if (array[i] < result.min) result.min = array[i];
        if (array[i] > result.max) result.max = array[i];
    }
    
    return result;
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    int timeout = 0;
    int with_files = 0;
    
    struct option long_options[] = {
        {"seed", required_argument, NULL, 0},
        {"array_size", required_argument, NULL, 0},
        {"pnum", required_argument, NULL, 0},
        {"timeout", required_argument, NULL, 0},
        {"by_files", no_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}
    };
    
    int option_index = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "f", long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                if (strcmp(long_options[option_index].name, "seed") == 0) {
                    seed = atoi(optarg);
                } else if (strcmp(long_options[option_index].name, "array_size") == 0) {
                    array_size = atoi(optarg);
                } else if (strcmp(long_options[option_index].name, "pnum") == 0) {
                    pnum = atoi(optarg);
                } else if (strcmp(long_options[option_index].name, "timeout") == 0) {
                    timeout = atoi(optarg);
                }
                break;
            case 'f':
                with_files = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s --seed <num> --array_size <num> --pnum <num> [--timeout <seconds>] [--by_files]\n", 
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    if (seed == -1 || array_size == -1 || pnum == -1) {
        fprintf(stderr, "Usage: %s --seed <num> --array_size <num> --pnum <num> [--timeout <seconds>] [--by_files]\n", 
                argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int *array = malloc(array_size * sizeof(int));
    GenerateArray(array, array_size, seed);
    
    int pipefd[pnum][2];
    if (!with_files) {
        for (int i = 0; i < pnum; i++) {
            if (pipe(pipefd[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    if (timeout > 0) {
        signal(SIGALRM, alarm_handler);
        alarm(timeout);
    }
    
    pid_t child_pids[pnum];
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    for (int i = 0; i < pnum; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            unsigned int chunk_size = array_size / pnum;
            unsigned int begin = i * chunk_size;
            unsigned int end = (i == pnum - 1) ? array_size : (i + 1) * chunk_size;
            
            struct MinMax result = GetMinMax(array, begin, end);
            
            if (with_files) {
                char filename[256];
                snprintf(filename, sizeof(filename), "minmax_%d.txt", i);
                FILE *f = fopen(filename, "w");
                if (f) {
                    fprintf(f, "%d %d\n", result.min, result.max);
                    fclose(f);
                }
            } else {
                close(pipefd[i][0]);
                write(pipefd[i][1], &result, sizeof(struct MinMax));
                close(pipefd[i][1]);
            }
            
            free(array);
            exit(0);
        } else {
            child_pids[i] = pid;
            if (!with_files) {
                close(pipefd[i][1]);
            }
        }
    }
    
    struct MinMax global_min_max;
    global_min_max.min = array[0];
    global_min_max.max = array[0];
    
    int active_children = pnum;
    int children_killed = 0;
    
    while (active_children > 0) {
        if (timeout > 0 && timeout_occurred) {
            printf("\nTimeout occurred! Killing remaining child processes...\n");
            for (int i = 0; i < pnum; i++) {
                if (child_pids[i] > 0) {
                    kill(child_pids[i], SIGKILL);
                    children_killed++;
                }
            }
            timeout_occurred = 0;
        }
        
        int status;
        pid_t finished_pid = waitpid(-1, &status, WNOHANG);
        
        if (finished_pid > 0) {
            active_children--;
            
            for (int i = 0; i < pnum; i++) {
                if (child_pids[i] == finished_pid) {
                    child_pids[i] = -1;
                    
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                        struct MinMax local_result;
                        
                        if (with_files) {
                            char filename[256];
                            snprintf(filename, sizeof(filename), "minmax_%d.txt", i);
                            FILE *f = fopen(filename, "r");
                            if (f) {
                                fscanf(f, "%d %d", &local_result.min, &local_result.max);
                                fclose(f);
                                remove(filename);
                            }
                        } else {
                            read(pipefd[i][0], &local_result, sizeof(struct MinMax));
                            close(pipefd[i][0]);
                        }
                        
                        if (local_result.min < global_min_max.min) 
                            global_min_max.min = local_result.min;
                        if (local_result.max > global_min_max.max) 
                            global_min_max.max = local_result.max;
                    }
                    break;
                }
            }
        } else if (finished_pid == 0) {
            usleep(10000);
        }
    }
    
    gettimeofday(&end_time, NULL);
    long elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000000L +
                        (end_time.tv_usec - start_time.tv_usec);
    
    printf("Min: %d\n", global_min_max.min);
    printf("Max: %d\n", global_min_max.max);
    printf("Elapsed time: %ld us (%.6f seconds)\n", 
           elapsed_time, elapsed_time / 1000000.0);
    
    if (children_killed > 0) {
        printf("Warning: %d child processes were killed due to timeout\n", 
               children_killed);
    }
    
    free(array);
    return 0;
}
