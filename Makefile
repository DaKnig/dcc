SRCDIR := src
OBJS := main.o tokenizer.o pratt.o
CFLAGS :=-O0 -ggdb -Wall -Wextra -Wshadow -Wcast-qual \
         -Wstrict-aliasing=1 -Wswitch-enum -Wstrict-prototypes \
	 -Wundef -Wpointer-arith -Wformat-security -Winit-self \
	 -Wwrite-strings -Wredundant-decls -Wno-unused

CC ?= gcc

all: main

test: main
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./main

main: $(OBJS) Makefile
	$(CC) $(CFLAGS) -o main $(OBJS)

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean test
clean:
	rm *.o
	rm main
