CC = g++
CFLAGS = -Wall -g -ggdb
EXECS = Tests/run

all: clean lz77 tests runTests

lz77: LZ77COMP LZ77DECOMP

LZ77COMP: LZ77/LZ77COMP.cpp
	@echo "Building LZ77 compresion..."
	@$(CC) $(CFLAGS) -c LZ77/LZ77COMP.cpp -o LZ77/LZ77COMP.o

LZ77DECOMP: LZ77/LZ77DECOMP.cpp
	@echo "Building LZ77 decompresion..."
	@$(CC) $(CFLAGS) -c LZ77/LZ77DECOMP.cpp -o LZ77/LZ77DECOMP.o

tests: Tests/run.cpp lz77
	@echo "Building Tests..."
	@$(CC) $(CFLAGS) -o Tests/run LZ77/LZ77COMP.o LZ77/LZ77DECOMP.o Tests/run.cpp -I LZ77

runTests:
	@echo "Running tests...\n"
	@./$(EXECS)

clean:
	@echo "Cleaning..."
	@rm -f *.o */*.o */*.lzip *.lzip $(EXECS)