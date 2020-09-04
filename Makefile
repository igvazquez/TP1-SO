CC = gcc
CFLAGS = -pedantic -Wall -std=c99 -pthread -lrt -g
RM = rm -f

SOURCES = $(wildcard *.c)
BINS = $(SOURCES:.c=.out)

TEST = test/*

all: $(BINS)

%.out: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(BINS)

.PHONY: all clean