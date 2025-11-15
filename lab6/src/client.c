#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <inttypes.h>  // ДОБАВЛЕНО: для PRIu64

#include "utils.h"

// Структура для хранения информации о сервере
struct Server {
  char ip[255];
  int port;
};

// Структура для передачи аргументов в поток клиента
struct ClientThreadArgs {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
};

// Функция потока для работы с одним сервером
void *ClientThread(void *args) {
  struct ClientThreadArgs *targs = (struct ClientThreadArgs *)args;
  
  // Получаем информацию о хосте по имени/IP
  struct hostent *hostname = gethostbyname(targs->server.ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", targs->server.ip);
    targs->result = 1;
    return NULL;
  }

  // Настраиваем структуру адреса сервера
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(targs->server.port);
  memcpy(&server.sin_addr, hostname->h_addr, hostname->h_length);

  // Создаём TCP сокет
  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    targs->result = 1;
    return NULL;
  }

  // Подключаемся к серверу
  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection failed to %s:%d\n", targs->server.ip, targs->server.port);
    close(sck);
    targs->result = 1;
    return NULL;
  }

  // Формируем задачу для сервера: begin, end, mod
  char task[sizeof(uint64_t) * 3];
  memcpy(task, &targs->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &targs->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &targs->mod, sizeof(uint64_t));

  // Отправляем задачу серверу
  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send failed\n");
    close(sck);
    targs->result = 1;
    return NULL;
  }

  // Получаем ответ от сервера
  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Receive failed\n");
    close(sck);
    targs->result = 1;
    return NULL;
  }

  // Извлекаем результат из ответа
  memcpy(&targs->result, response, sizeof(uint64_t));
  
  close(sck);
  return NULL;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; // 255 - стандартный размер для путей

  // Парсим аргументы командной строки с помощью getopt_long
  while (true) {
    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        break;
      case 2:
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // Читаем список серверов из файла
  FILE *file = fopen(servers, "r");
  if (!file) {
    fprintf(stderr, "Cannot open file %s\n", servers);
    return 1;
  }

  // Подсчитываем количество серверов в файле
  unsigned int servers_num = 0;
  char line[256];
  while (fgets(line, sizeof(line), file)) {
    servers_num++;
  }
  rewind(file);

  if (servers_num == 0) {
    fprintf(stderr, "No servers found in file\n");
    fclose(file);
    return 1;
  }

  // Выделяем память для массива серверов
  struct Server *to = malloc(sizeof(struct Server) * servers_num);
  
  // Читаем IP-адреса и порты из файла (формат: ip:port)
  for (unsigned int i = 0; i < servers_num; i++) {
    if (fgets(line, sizeof(line), file)) {
      char *colon = strchr(line, ':');
      if (colon) {
        *colon = '\0';
        strncpy(to[i].ip, line, sizeof(to[i].ip) - 1);
        to[i].port = atoi(colon + 1);
      }
    }
  }
  fclose(file);

  // Разбиваем диапазон [1, k] на части для каждого сервера
  pthread_t threads[servers_num];
  struct ClientThreadArgs thread_args[servers_num];
  
  uint64_t range_per_server = k / servers_num;
  uint64_t remainder = k % servers_num;

  for (unsigned int i = 0; i < servers_num; i++) {
    thread_args[i].server = to[i];
    thread_args[i].mod = mod;
    
    // Вычисляем диапазон для каждого сервера
    thread_args[i].begin = i * range_per_server + 1;
    thread_args[i].end = (i + 1) * range_per_server;
    
    // Последнему серверу добавляем остаток от деления
    if (i == servers_num - 1) {
      thread_args[i].end += remainder;
    }
    
    thread_args[i].result = 1;

    // Создаём поток для параллельной работы с каждым сервером
    if (pthread_create(&threads[i], NULL, ClientThread, &thread_args[i])) {
      fprintf(stderr, "Error: pthread_create failed!\n");
      free(to);
      return 1;
    }
  }

  // Ждём завершения всех потоков и объединяем результаты
  uint64_t answer = 1;
  for (unsigned int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    answer = MultModulo(answer, thread_args[i].result, mod);
  }

  // ИСПРАВЛЕНО: используем PRIu64 для портируемого вывода uint64_t
  printf("answer: %" PRIu64 "\n", answer);

  free(to);
  return 0;
}
