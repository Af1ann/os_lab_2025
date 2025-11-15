# Makefile для лабораторной работы №4
# Задания 1, 3, 6

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
PTHREAD_FLAGS = -pthread

# Все программы
TARGETS = parallel_min_max process_memory parallel_sum

# Цель по умолчанию - собрать все
all: $(TARGETS)

# Задание 1: parallel_min_max с таймаутом
parallel_min_max: parallel_min_max.c
	$(CC) $(CFLAGS) -o parallel_min_max parallel_min_max.c

# Задание 3: демонстрация памяти процесса
process_memory: process_memory.c
	$(CC) $(CFLAGS) -o process_memory process_memory.c

# Задание 6: parallel_sum (задание 5+6)
parallel_sum: parallel_sum.c
	$(CC) $(CFLAGS) $(PTHREAD_FLAGS) -o parallel_sum parallel_sum.c

# Очистка
clean:
	rm -f $(TARGETS) *.o minmax_*.txt

# Тесты
test: all
	@echo "=== Тест задания 1: parallel_min_max ==="
	./parallel_min_max --seed 42 --array_size 1000000 --pnum 4
	@echo "\n=== Тест задания 3: process_memory ==="
	./process_memory
	@echo "\n=== Тест задания 6: parallel_sum ==="
	./parallel_sum --threads_num 4 --array_size 1000000 --seed 42

.PHONY: all clean test
