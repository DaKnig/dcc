
OBJS := main.o tokenizer.o pratt.o
SRCS := main.c tokenizer.c pratt.c
CFLAGS := -O0 -ggdb -Wall -Wextra # -Werror

CC := gcc

#.SUFFIXES:

all: main

test: main
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./main

main: $(OBJS) Makefile
	$(CC) $(CFLAGS) -o main $(OBJS)


%.o: %.c | %.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean test
clean:
	rm *.o
	rm main
