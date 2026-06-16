CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2
LDFLAGS = -lrt -lm

all: zadanie1

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c

zadanie1: common.o zadanie1.c
	$(CC) $(CFLAGS) zadanie1.c common.o -o zadanie1 $(LDFLAGS)

clean:
	rm -f *.o zadanie1 *.jpg *.log