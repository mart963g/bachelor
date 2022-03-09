CC = g++
CFLAGS = -Wall -g -ggdb

all: clean lz77 run

lz77:
	$(CC) $(CFLAGS) -c LZ77/LZ77.cpp -o LZ77/LZ77.o

run:
	$(CC) $(CFLAGS) -o run LZ77/LZ77.o run.cpp

clean:
	rm -f *.o */*.o run