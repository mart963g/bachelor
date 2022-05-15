CC = g++
CFLAGS = -Wall -g -ggdb
EXECS = Tests/Execs/run Tests/Execs/flak
TESTS = Hello.txt shakespeare.txt Tester.png speech.wav forest.wav
TEXT_FILES = Hello.txt shakespeare.txt
NON_TEXT_FILES = Tester.png speech.wav forest.wav
DIFF_TEXT = Tests/Files/$(file).trash Tests/Decompressed/$(file).trash

all: clean lz77 flak tests runLZ77 diff cleanFiles runFlak secondDiff

hello: clean lz77 runHello

forest: clean flak runForest

speech: clean flak runSpeech

lz77: LZ77COMP LZ77DECOMP

flak: RICECODER FLAKCOMP FLAKDECOMP

flakTests: clean flak flakTest runFlak diff

LZ77COMP: LZ77/LZ77COMP.cpp
	@echo "Building LZ77 compresion..."
	@$(CC) $(CFLAGS) -c LZ77/LZ77COMP.cpp -o LZ77/LZ77COMP.o

LZ77DECOMP: LZ77/LZ77DECOMP.cpp
	@echo "Building LZ77 decompresion..."
	@$(CC) $(CFLAGS) -c LZ77/LZ77DECOMP.cpp -o LZ77/LZ77DECOMP.o

FLAKCOMP: FLAK/FLAKCOMP.cpp
	@echo "Building FLAK compression..."
	@$(CC) $(CFLAGS) -c FLAK/FLAKCOMP.cpp -o FLAK/FLAKCOMP.o

FLAKDECOMP: FLAK/FLAKDECOMP.cpp
	@echo "Building FLAK decompresion..."
	@$(CC) $(CFLAGS) -c FLAK/FLAKDECOMP.cpp -o FLAK/FLAKDECOMP.o

RICECODER:	RICE/RICECODER.cpp
	@echo "Building Rice coder..."
	@$(CC) $(CFLAGS) -c RICE/RICECODER.cpp -o RICE/RICECODER.o


tests: Tests/Execs/run.cpp Tests/Execs/flak.cpp lz77 flak
	@echo "Building Tests..."
	@$(CC) $(CFLAGS) -o Tests/Execs/run LZ77/LZ77COMP.o LZ77/LZ77DECOMP.o Tests/Execs/run.cpp -I LZ77
	@$(CC) $(CFLAGS) -o Tests/Execs/flak FLAK/FLAKCOMP.o FLAK/FLAKDECOMP.o Tests/Execs/flak.cpp -I FLAK

flakTest: Tests/Execs/flak.cpp flak
	@echo "Building Test..."
	@$(CC) $(CFLAGS) -o Tests/Execs/flak FLAK/FLAKCOMP.o FLAK/FLAKDECOMP.o Tests/Execs/flak.cpp -I FLAK

runLZ77: lz77 tests
	@echo "Running tests LZ77...\n"
	@./Tests/Execs/run ${TESTS}

runHello:
	@echo "Building Hello test..."
	@$(CC) $(CFLAGS) -o Tests/Execs/run LZ77/LZ77COMP.o LZ77/LZ77DECOMP.o Tests/Execs/run.cpp -I LZ77
	@echo "Running hello test...\n"
	@./Tests/Execs/run Hello.txt
	@echo "\nChecking that hello files are identical..."
	@diff Tests/Files/Hello.txt Tests/Decompressed/Hello.txt

flakHello: flak
	@echo "Building flak test..."
	@$(CC) $(CFLAGS) -o Tests/Execs/flak FLAK/FLAKCOMP.o FLAK/FLAKDECOMP.o Tests/Execs/flak.cpp -I FLAK
	@echo "Running flak Hello.txt test...\n"
	@./Tests/Execs/flak Hello.txt
	@echo "Checking that hello files are identical..."
	@diff Tests/Files/Hello.txt Tests/Decompressed/Hello.txt

flakShakespeare: flak
	@echo "Building flak test..."
	@$(CC) $(CFLAGS) -o Tests/Execs/flak FLAK/FLAKCOMP.o FLAK/FLAKDECOMP.o Tests/Execs/flak.cpp -I FLAK
	@echo "Running flak shakespeare.txt test...\n"
	@./Tests/Execs/flak shakespeare.txt
	@echo "\nChecking that shakespeare files are identical..."
	@diff Tests/Files/shakespeare.txt Tests/Decompressed/shakespeare.txt

flakPng: flak
	@echo "Building flak test..."
	@$(CC) $(CFLAGS) -o Tests/Execs/flak FLAK/FLAKCOMP.o FLAK/FLAKDECOMP.o Tests/Execs/flak.cpp -I FLAK
	@echo "Running flak PNG test...\n"
	@./Tests/Execs/flak Tester.png
	@echo "\nChecking that PNG files are identical..."
	@diff Tests/Files/Tester.png Tests/Decompressed/Tester.png

runForest: flak
	@echo "Building forest test..."
	@$(CC) $(CFLAGS) -o Tests/Execs/flak FLAK/FLAKCOMP.o FLAK/FLAKDECOMP.o Tests/Execs/flak.cpp -I FLAK
	@echo "Running forest test...\n"
	@./Tests/Execs/flak forest.wav
	@echo "\nGenerating hex dump files..."
	@xxd Tests/Files/forest.wav > Tests/Files/forest.wav.trash
	@xxd Tests/Decompressed/forest.wav > Tests/Decompressed/forest.wav.trash
	@echo "Checking that forest files are identical..."
	@diff Tests/Files/forest.wav.trash Tests/Decompressed/forest.wav.trash
	@rm Tests/Files/forest.wav.trash Tests/Decompressed/forest.wav.trash

runSpeech: flak
	@echo "Building speech test..."
	@$(CC) $(CFLAGS) -o Tests/Execs/flak FLAK/FLAKCOMP.o RICE/RICECODER.o FLAK/FLAKDECOMP.o Tests/Execs/flak.cpp -I RICE -I FLAK
	@echo "Running speech test...\n"
	@./Tests/Execs/flak speech.wav
	@ ls -l Tests/Files/speech.wav Tests/Compressed/speech.wav.flak
	# @echo "\nGenerating hex dump files..."
	# @xxd Tests/Files/speech.wav > Tests/Files/speech.wav.trash
	# @xxd Tests/Decompressed/speech.wav > Tests/Decompressed/speech.wav.trash
	# @echo "Checking that speech files are identical..."
	# @diff Tests/Files/speech.wav.trash Tests/Decompressed/speech.wav.trash
	# @rm Tests/Files/speech.wav.trash Tests/Decompressed/speech.wav.trash
	
	
#@echo "Sizes:"
#@ls -l Tests/Files/speech.wav Tests/Compressed/speech.wav.flak
	
runFlak: flak
	@echo "Running tests FLAK...\n"
	@./Tests/Execs/flak ${TESTS}

diff:
	@echo "\nChecking that text files are identical..."
	@$(foreach file,$(TEXT_FILES), echo Checking $(file)... && diff Tests/Files/$(file) Tests/Decompressed/$(file); )
	@echo "\nGenerating non text file hex dumps..."
	@$(foreach file,$(NON_TEXT_FILES), xxd Tests/Files/$(file) > Tests/Files/$(file).trash; xxd Tests/Decompressed/$(file) > Tests/Decompressed/$(file).trash;)
	@echo "Checking that non text files are identical..."
	@$(foreach file,$(NON_TEXT_FILES), echo "Checking $(file)..." && diff $(DIFF_TEXT);)
	@rm -f */*/*.trash 

secondDiff:
	@echo "\nChecking that text files are identical..."
	@$(foreach file,$(TEXT_FILES), echo Checking $(file)... && diff Tests/Files/$(file) Tests/Decompressed/$(file); )
	@echo "\nGenerating non text file hex dumps..."
	@$(foreach file,$(NON_TEXT_FILES), xxd Tests/Files/$(file) > Tests/Files/$(file).trash; xxd Tests/Decompressed/$(file) > Tests/Decompressed/$(file).trash;)
	@echo "Checking that non text files are identical..."
	@$(foreach file,$(NON_TEXT_FILES), echo "Checking $(file)..." && diff $(DIFF_TEXT);)
	@rm -f */*/*.trash 

sizes:
	@echo "Sizes of original and decompressed files:\n"
	@$(foreach file, $(TESTS), ls -l Tests/Files/$(file) Tests/Compressed/$(file).flak Tests/Decompressed/$(file);)

clean:
	@echo "Cleaning..."
	@rm -f *.o */*.o */*/*.lzip */*.lzip */*/*.flak */*.flak Tests/Decompressed/* Tests/Compressed/* */*/*.trash $(EXECS) 

cleanFiles:
	@echo "Cleaning compressed files..."
	@rm Tests/Decompressed/* Tests/Compressed/*