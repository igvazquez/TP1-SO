CC = gcc
CFLAGS = -pedantic -Wall -std=c99 -g
CLIBS = -pthread -lrt

RM = rm -f

SOURCES = $(wildcard *.c)
BINS = $(SOURCES:.c=.out)

TEST = test/*

all: $(BINS)

%.out: %.c
	$(CC) $(CFLAGS) $^ -o $@ $(CLIBS)

clean:
	$(RM) $(BINS)

.PHONY: all clean