CC = g++
CFLAGS = -Wall -g -ggdb
EXECS = Tests/Execs/run
TESTS = Hello.txt shakespeare.txt Tester.png forest.wav
TEXT_FILES = Hello.txt shakespeare.txt
NON_TEXT_FILES = Tester.png forest.wav
DIFF_TEXT = Tests/Files/$(file).trash Tests/Decompressed/$(file).trash

all: clean lz77 tests runTests diff

lz77: LZ77COMP LZ77DECOMP

LZ77COMP: LZ77/LZ77COMP.cpp
	@echo "Building LZ77 compresion..."
	@$(CC) $(CFLAGS) -c LZ77/LZ77COMP.cpp -o LZ77/LZ77COMP.o

LZ77DECOMP: LZ77/LZ77DECOMP.cpp
	@echo "Building LZ77 decompresion..."
	@$(CC) $(CFLAGS) -c LZ77/LZ77DECOMP.cpp -o LZ77/LZ77DECOMP.o

tests: Tests/Execs/run.cpp lz77
	@echo "Building Tests..."
	@$(CC) $(CFLAGS) -o Tests/Execs/run LZ77/LZ77COMP.o LZ77/LZ77DECOMP.o Tests/Execs/run.cpp -I LZ77

runTests: tests
	@echo "Running tests...\n"
	@./$(EXECS) ${TESTS}


diff:
	@echo "\nChecking that text files are identical..."
	@$(foreach file,$(TEXT_FILES), echo Checking $(file)... && diff Tests/Files/$(file) Tests/Decompressed/$(file); )
	@echo "\nGenerating non text file hex dumps..."
	@$(foreach file,$(NON_TEXT_FILES), xxd Tests/Files/$(file) > Tests/Files/$(file).trash; xxd Tests/Decompressed/$(file) > Tests/Decompressed/$(file).trash;)
	@echo "Checking that non text files are identical..."
	@$(foreach file,$(NON_TEXT_FILES), echo "Checking $(file)..." && diff $(DIFF_TEXT);)
	@rm -f */*/*.trash 


clean:
	@echo "Cleaning..."
	@rm -f *.o */*.o */*/*.lzip */*.lzip $(EXECS) 