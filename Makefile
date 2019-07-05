SRCDIR := src
TESTSDIR := test
BINDIR := bin
OBJS := main.o lexer.o tokenizer.o pratt.o token.o
CFLAGS :=-O0 -ggdb -Wall -Wextra -Wshadow -Wcast-qual \
    	-Wstrict-aliasing=1 -Wswitch-enum -Wstrict-prototypes \
		-Wundef -Wpointer-arith -Wformat-security -Winit-self \
		-Wwrite-strings -Wredundant-decls -Wno-unused

TESTS := $(BINDIR)/test_lexer_1 $(BINDIR)/test_acc_1 $(BINDIR)/test_lexer_2

TESTER := ./src/tester.py

CC ?= gcc

all: $(BINDIR)/main unit-tests

memtest: $(BINDIR)/main
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./main

$(BINDIR)/main: $(OBJS) Makefile
	$(CC) $(CFLAGS) -o $@ $(OBJS)

unit-tests: $(TESTER) $(TESTS)
	$(TESTER) $(TESTS)

$(BINDIR)/test_lexer_%: test/test_lexer_%.c lexer.o token.o
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)" 

$(BINDIR)/test_acc_1: test/test_acc_1.c decl_parser.o lexer.o token.o
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)" 

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean memtest test-unit run
clean:
	rm -f *.o
	rm -f main
	rm -f bin/*
run:$(BINDIR)/main
	$(BINDIR)/main
