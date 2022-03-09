CC = g++
CFLAGS = -Wall -g -ggdb

all: lz77

lz77:
	$(CC) $(CFLAGS) LZ77/LZ77.cpp -o LZ77/lz77



clean:
	rm LZ77/lz77