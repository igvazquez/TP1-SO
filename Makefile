CC = gcc
CFLAGS = -pedantic -Wall -std=c99 -pthread -lrt -g
RM = rm -f

SOURCES = $(wildcard *.c)
BINS = $(SOURCES:.c=.out)

CPPANS = $(SOURCES:.c=.cppout)	

TF = test/sat/*

all: $(BINS)

%.out: %.c
	$(CC) $^ -o $@ $(CFLAGS)

clean: clean_check
	$(RM) $(BINS)

clean_check:
	$(RM) $(CPPANS) *.valout report.tasks

check: clean $(CPPANS) 
	./pvs.sh

test: all $(CPPANS)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=master.valout ./master.out $(TF) 2> master.valout | valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=view.valout ./view.out 2> view.valout > /dev/null

%.cppout: %.c
	cppcheck --quiet --enable=all --force --inconclusive  $^ 2> $@


.PHONY: all clean check clean_check