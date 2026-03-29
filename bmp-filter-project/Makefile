CC = gcc
CFLAGS = -ansi -pedantic -Wall -Werror
OBJ = main.o bmp.o parser.o filters.o

bmpfilter: $(OBJ)
	$(CC) $(CFLAGS) -o bmpfilter $(OBJ)

main.o: main.c bmp.h parser.h filters.h
bmp.o: bmp.c bmp.h
parser.o: parser.c parser.h
filters.o: filters.c filters.h bmp.h

clean:
	rm -f *.o bmpfilter