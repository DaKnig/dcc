SRCDIR := src
TESTSDIR := test
BINDIR := bin
FUZZDIR := fuzz

OBJS := pratt.o log.o atom.o statement_parser.o tokenizer.o \
		decl_parser.o context.o top_level_parser.o

OBJS := $(addprefix $(SRCDIR)/,$(OBJS))

CFLAGS := -Isrc\
	-O0 -ggdb -Wall -Werror -Wextra -Wshadow -Wcast-qual \
		-Wstrict-aliasing=1 -Wswitch -Wstrict-prototypes \
		-Wundef -Wpointer-arith -Wformat-security -Winit-self \
		-Wredundant-decls -Wno-unused -fmax-errors=2 \
#		-fsanitize=address -fno-omit-frame-pointer

TESTS :=  $(BINDIR)/test_logging $(BINDIR)/test_atom $(BINDIR)/simple_decl \
		$(BINDIR)/tokenizer_test

TESTER := ./tester.py

CC := gcc

all: $(BINDIR)/main #unit-tests

memtest: $(BINDIR)/main
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./$^

$(BINDIR)/%: $(OBJS) $(SRCDIR)/%.o | $(BINDIR)/
	$(CC) $(CFLAGS) -o $@ $^

unit-tests: $(TESTER) $(TESTS)
	$(TESTER) $(TESTS)

$(BINDIR)/test_logging: test/test_logging.c log.o | $(BINDIR)/
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

$(BINDIR)/tokenizer_test: test/tokenizer_test.c tokenizer.o log.o context.o | $(BINDIR)/
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

$(BINDIR)/simple_decl: test/simple_decl.c $(OBJS) | $(BINDIR)/
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

$(BINDIR)/test_atom: test/test_atom.c atom.o log.o | $(BINDIR)/
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

$(BINDIR)/fuzz_lexer: CC := afl-gcc # Difficult not to hard code this
$(BINDIR)/fuzz_lexer: $(FUZZDIR)/fuzz_lexer.c lexer.o token.o atom.o log.o | $(BINDIR)/
	$(CC) $(CFLAGS) -o $@ $^ -I"$(SRCDIR)"

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%/:
	mkdir -p $@

.PHONY: clean memtest unit-tests fuzz run format test
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
	rm -f src/*.o
	rm -f $(TESTSDIR)/*/main
run:$(BINDIR)/main
	$(BINDIR)/main

format:
	clang-format -i src/*.c

test: ./test/run_tests.sh all
	./test/run_tests.sh
