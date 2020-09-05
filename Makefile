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

test: clean $(CPPANS) 
	./pvs.sh
	valgrind ./master.out $(TF) 2> master.valout | valgrind ./view.out 2> view.valout > /dev/null

clean_test:
	$(RM) $(CPPANS) *.valout report.tasks

%.cppout: %.c
	cppcheck --quiet --enable=all --force --inconclusive  $^ 2> $@

.PHONY: all clean test clean_test