CC = g++
CFLAGS = -Wall -g -ggdb
EXECS = Tests/run

all: clean lz77 run runTests

lz77: LZ77/LZ77.cpp
	@echo "Building LZ77..."
	@$(CC) $(CFLAGS) -c LZ77/LZ77.cpp -o LZ77/LZ77.o

run: Tests/run.cpp LZ77/LZ77.o LZ77/LZ77.cpp
	@echo "Building Tests..."
	@$(CC) $(CFLAGS) -o Tests/run LZ77/LZ77.o Tests/run.cpp -I LZ77

runTests:
	@echo "Running tests...\n"
	@./Tests/run

clean:
	@echo "Cleaning..."
	@rm -f *.o */*.o $(EXECS)