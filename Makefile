SRCDIR := src
TESTSDIR := test
BINDIR := bin
FUZZDIR := fuzz

OBJS := pratt.o log.o atom.o statement_parser.o new_tokenizer.o \
		decl_parser.o context.o
CFLAGS :=-Og -ggdb -Wall -Werror -Wextra -Wshadow -Wcast-qual \
		-Wstrict-aliasing=1 -Wswitch-enum -Wstrict-prototypes \
		-Wundef -Wpointer-arith -Wformat-security -Winit-self \
		-Wwrite-strings -Wredundant-decls -Wno-unused

TESTS :=  $(BINDIR)/test_logging $(BINDIR)/test_atom \
		$(BINDIR)/tokenizer_test $(BINDIR)/simple_decl

TESTER := ./src/tester.py

CC ?= gcc

all: $(BINDIR)/main #unit-tests

memtest: $(BINDIR)/main
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./main

$(BINDIR)/main: $(OBJS) Makefile
	$(CC) $(CFLAGS) -o $@ $(OBJS)

unit-tests: $(TESTER) $(TESTS)
	$(TESTER) $(TESTS)

$(BINDIR)/test_lexer_%: test/test_lexer_%.c lexer.o token.o atom.o log.o
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

$(BINDIR)/tokenizer_test: test/tokenizer_test.c new_tokenizer.o log.o context.o
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

$(BINDIR)/test_logging: test/test_logging.c log.o
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

$(BINDIR)/test_atom: test/test_atom.c atom.o log.o
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

$(BINDIR)/fuzz_lexer: CC := afl-gcc # Difficult not to hard code this
$(BINDIR)/fuzz_lexer: $(FUZZDIR)/fuzz_lexer.c lexer.o token.o atom.o log.o
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

$(BINDIR)/test_sym_table: test/test_sym_table.c symbol_table.o decl_parser.o lexer.o
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean memtest unit-tests fuzz run
fuzz: $(BINDIR)/fuzz_lexer
	# Usage:
	# FUZZ_OUT=outdir make fuzz
	# If you have a SSD, you should ideally place outdir on a ramdisk:
	# cd /tmp && mkdir afl
	# chmod 777 afl
	# sudo mount -t tmpfs -o size=256M tmpfs /tmp/afl
	# mkdir afl
	# Then back in the project directory:
	# FUZZ_OUT=/tmp/afl make fuzz
	afl-fuzz -i $(FUZZDIR)/in -o $(FUZZ_OUT) ./$(BINDIR)/fuzz_lexer

clean:
	rm -f *.o
	rm -f main
	rm -f bin/*
run:$(BINDIR)/main
	$(BINDIR)/main
