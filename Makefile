CC = gcc
CFLAGS = -ansi -pedantic -Wall -Werror

all: bmpfilter

bmpfilter: main.o bmp.o fsm.o parser.o filters.o
	$(CC) $(CFLAGS) -o bmpfilter main.o bmp.o fsm.o parser.o filters.o

main.o: main.c bmp.h fsm.h parser.h filters.h
	$(CC) $(CFLAGS) -c main.c

bmp.o: bmp.c bmp.h
	$(CC) $(CFLAGS) -c bmp.c

fsm.o: fsm.c fsm.h bmp.h
	$(CC) $(CFLAGS) -c fsm.c

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c

filters.o: filters.c filters.h bmp.h parser.h
	$(CC) $(CFLAGS) -c filters.c

clean:
	rm -f *.o bmpfilter