SRCDIR := src
TESTSDIR := test
BINDIR := bin
OBJS := main.o lexer.o tokenizer.o pratt.o
CFLAGS :=-O0 -ggdb -Wall -Wextra -Wshadow -Wcast-qual \
         -Wstrict-aliasing=1 -Wswitch-enum -Wstrict-prototypes \
	 -Wundef -Wpointer-arith -Wformat-security -Winit-self \
	 -Wwrite-strings -Wredundant-decls -Wno-unused

TESTS := bin/test_lexer_1

TESTER := ./src/tester.py

CC ?= gcc

all: $(BINDIR)/main unit-tests

memtest: $(BINDIR)/main
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./main

$(BINDIR)/main: $(OBJS) Makefile
	$(CC) $(CFLAGS) -o main $(OBJS)

unit-tests: $(TESTS) $(TESTER)
	$(TESTER) $(TESTS)

$(BINDIR)/test_lexer_1: test/test_lexer_1.c lexer.o
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)" 

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean test test-unit
clean:
	rm -f *.o
	rm -f main
	rm -f bin/*
