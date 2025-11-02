# Makefile for Lab 5
CC = gcc
CFLAGS = -std=c99 -Wall -Werror -pedantic
LDFLAGS = -lpthread

all: mutex factorial deadlock deadlock_detailed deadlock_prevention

mutex: mutex.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

factorial: factorial.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

deadlock: deadlock.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

deadlock_detailed: deadlock_detailed.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

deadlock_prevention: deadlock_prevention.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f mutex factorial deadlock deadlock_detailed deadlock_prevention

.PHONY: all clean