SRCDIR := src
TESTSDIR := test
OBJS := main.o lexer.o tokenizer.o pratt.o
CFLAGS :=-O0 -ggdb -Wall -Wextra -Wshadow -Wcast-qual \
         -Wstrict-aliasing=1 -Wswitch-enum -Wstrict-prototypes \
	 -Wundef -Wpointer-arith -Wformat-security -Winit-self \
	 -Wwrite-strings -Wredundant-decls -Wno-unused

CC ?= gcc

all: main unit-tests

test: main
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./main

main: $(OBJS) Makefile
	$(CC) $(CFLAGS) -o main $(OBJS)

unit-tests: test_lexer_1

test_lexer_1: test/test_lexer_1.c lexer.o
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)" 

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean test test-unit
clean:
	rm *.o
	rm main
