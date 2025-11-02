CC = gcc
CFLAGS = -std=c99 -Wall -Werror -pedantic

all: sequential_min_max parallel_min_max exec_sequential

sequential_min_max: sequential_min_max.o find_min_max.o utils.o
	$(CC) $(CFLAGS) $^ -o $@

parallel_min_max: parallel_min_max.o find_min_max.o utils.o
	$(CC) $(CFLAGS) $^ -o $@

exec_sequential: exec_sequential.o
	$(CC) $(CFLAGS) $^ -o $@

sequential_min_max.o: sequential_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c $< -o $@

parallel_min_max.o: parallel_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c $< -o $@

exec_sequential.o: exec_sequential.c
	$(CC) $(CFLAGS) -c $< -o $@

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c $< -o $@

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o sequential_min_max parallel_min_max exec_sequential min_max_*.txt

.PHONY: all clean