#include "LZ77.h"

using namespace std;

void LZ77COMP::compressFile(string file_name) {
    if (this->initialiseCompression(file_name) == 0 ) {
        this->compressAfterInitialisation();
    } 
    this->input_file.close();
    this->output_file.close();
}

void LZ77COMP::compressFile(string file_name, string destination_file) {
    if (this->initialiseCompression(file_name, destination_file) == 0) {
        this->compressAfterInitialisation();
    }
    this->input_file.close();
    this->output_file.close();

}

int LZ77COMP::initialiseCompression(string file_name) {
    // Open a name with filename.lzip
    this->input_file.open(file_name, ios::binary);
    this->output_file.open(file_name + ".lzip", ios::app | ios::binary);

    return this->initialiseBuffers();
}

int LZ77COMP::initialiseCompression(string file_name, string destination_file) {
    // Open a name with filename.zip
    this->input_file.open(file_name, ios::binary);
    this->output_file.open(destination_file + ".lzip", ios::app | ios::binary);

    return this->initialiseBuffers();
}

int LZ77COMP::initialiseBuffers(int history_size, int lookahead_size) {
    this->history_buffer_size = history_size;
    for(int i = this->lookahead_buffer_start; i < lookahead_size; i++){
        unsigned char temp = this->input_file.get();
        if (this->input_file.eof()) {
            this->finishCompression();
            // Skip the rest of the code
            return 1;
        }
        this->lookahead_buffer.push_back(temp);
        this->lookahead_buffer_end++;
    }
    // printf("Exited for loop with L buffer start: %d; L buffer end: %d\n", this->lookahead_buffer_start, this->lookahead_buffer_end);
    // printf("Lookahead buffer: '%s'\n\n", this->lookahead_buffer.data());
    int move = 0;
    int remainder = 0;
    /* Runs until history buffer is full */
    while(this->history_buffer_end < history_size) {
        // cout << "----------------------------------\n";
        struct token token = this->searchForMatch();
        this->writeTokenToOutputFile(token);
        // printf("Offset: %u; Length: %u\n", token.offset, token.length);

        // If no match was found, move 1 char, otherwise move the length of the match
        move = (token.offset == 0 ? 1 : (int) token.length);
        // cout << "Move: " << move << "\n";

        if ((remainder = this->moveBuffers(move)) != 0) {
            // If EOF we still need to increment the buffer indexes, before calling finishCompression()
            // printf("Move = %d; Remainder = %d\n\n", move, remainder);
            int actual_move = (remainder != -1) ? (move - remainder) : 1;
            // Put the remainin bytes in the history buffer
            this->pumpHistoryBuffer(actual_move);
            this->lookahead_buffer_start += actual_move;
            this->history_buffer_end += actual_move;
            // Set start to preserve history buffer size otherwise keep start at 0
            this->history_buffer_start = this->history_buffer_end >= history_size ? this->history_buffer_end - history_size : 0;
            this->finishCompression();
            return 1;
        }

        // Set start to preserve history buffer size otherwise keep start at 0
        this->history_buffer_start = this->history_buffer_end >= history_size ? this->history_buffer_end - history_size : 0;
        // printf("History start: %d; History end: %d\n", this->history_buffer_start, this->history_buffer_end);
        // printf("Lookahead start: %d; Lookahead end: %d \n\n", this->lookahead_buffer_start, this->lookahead_buffer_end);
    }
    return 0;
}

void LZ77COMP::compressAfterInitialisation() {
    int move = 0;
    struct token token; 
    int remainder = 0;
    // printf("Is now inside compressAfterInitialisation function!\n\n\n");
    while ( (remainder = this->moveBuffers(move)) == 0) {
        token = this->searchForMatch();
        this->writeTokenToOutputFile(token);

        // If no match was found, move 1 char, otherwise move the length of the match
        move = (token.offset == 0 ? 1 : (int) token.length);
    }
    // cout << "Remainder: " << remainder << "\n";
    // cout << "Move: " << move << "\n";
    // Finish the moving of the buffers, without changing the end of the lookahead buffer
    int actual_move = (remainder != -1) ? (move - remainder) : move;
    // Fill the history buffer with the remaining bytes
    this->pumpHistoryBuffer(actual_move);
    this->lookahead_buffer_start += actual_move;
    this->history_buffer_start += actual_move;
    this->history_buffer_end += actual_move;
    // Deal with the remaining bytes of the file
    // printf("Now in the finishing part with remainder: %d and move: %d\n\n", remainder, move);
    this->finishCompression();
}

void LZ77COMP::finishCompression() {
    struct token token;
    int move = 0;
    while (this->lookahead_buffer_start < this->lookahead_buffer_end) {
        // printf("L buffer start: %d; L buffer end: %d\n", this->lookahead_buffer_start, this->lookahead_buffer_end);
        token = this->searchForMatch();
        this->writeTokenToOutputFile(token);

        // If no match was found, move 1 char, otherwise move the length of the match
        move = (token.offset == 0 ? 1 : (int) token.length);
        // Put the remainin bytes in the history buffer
        this->pumpHistoryBuffer(move);
        this->lookahead_buffer_start += move;
        this->history_buffer_start = this->history_buffer_end > history_buffer_size ?  this->history_buffer_start + move : this->history_buffer_start;
        this->history_buffer_end += move;
    }
}

void LZ77COMP::writeTokenToOutputFile(struct token token) {
    if (token.offset == 0) {
        struct nonMatchToken writeToken = this->getNonMatchToken(token);
        this->output_file.write(reinterpret_cast<char*>(&writeToken), sizeof(struct nonMatchToken));
    } else {
        struct matchToken writeToken = this->getMatchToken(token);
        this->output_file.write(reinterpret_cast<char*>(&writeToken), sizeof(struct matchToken));
    }
    // printf("Writing token to output file with offset: %u and length %u\n", token.offset, token.length);
    // printf("L buffer start: %d; L buffer end: %d\n", this->lookahead_buffer_start, this->lookahead_buffer_end);
    // printf("L buffer data: %s\n", this->lookahead_buffer.data());
    // printf("H buffer start: %d; H buffer end: %d\n", this->history_buffer_start, this->history_buffer_end);
    // printf("H buffer size: %lu\n", this->history_buffer.size());
    // printf("H buffer data: %s\n\n", this->history_buffer.data());
}

struct matchToken LZ77COMP::getMatchToken(struct token token) {
    struct matchToken writeToken;
    writeToken.non_match = 0;
    writeToken.offset = token.offset;
    writeToken.length = token.length;

    return writeToken;
}

struct nonMatchToken LZ77COMP::getNonMatchToken(struct token token) {
    struct nonMatchToken writeToken;
    writeToken.non_match = 1;
    writeToken.offset = token.offset;
    writeToken.length = token.length;

    return writeToken;
}

/* 
Start with first character in lookahead buffer, search backwards in history buffer. If there
is a match check if it is also a match for next char in lookahead buffer. If this is not the
case, save placement and length somewhere, and look in the rest,of the history buffer. 
When the history buffer start is reached, pick the longest closest match, and return it.    */

/* Finds and returns the longest match, closest to the lookahead buffer */
struct token LZ77COMP::searchForMatch() {
    int longest = 1; // We only want matches of length >= 2
    int closest = 0; // This default value does not matter
    
    // Small optimisation, so a pointer is not accessed everytime
    unsigned char first = this->lookahead_buffer[this->lookahead_buffer_start];
    
    // The default return value, if no match is found
    struct token match = {0, first};

    int current = this->history_buffer_end - 1;
    // Go from right to left
    while(current >= this->history_buffer_start) {
        // If a match is found, go from left to right
        if (this->history_buffer[current] == first) {
            // cout << "Match on first char in lookahead buffer: " << first << "\n";
            int temp = current + 1;
            // printf("Current index is: %d \n", current);
            
            // While we are not at the end at the history or lookahead buffer,
            // and the sequences continue matching
            while (temp < this->history_buffer_end && 
                   this->lookahead_buffer_start + (temp-current) < this->lookahead_buffer_end &&
                   this->history_buffer[temp] == this->lookahead_buffer[this->lookahead_buffer_start + (temp-current)])
            {
                // printf("Temp index is: %d; \t", temp);

                // If we found a longer match
                if ( (temp - current) + 1 > longest ) {
                    // The plus one is needed, because the difference is one lower than the length.
                    longest = temp - current + 1;
                    closest = this->history_buffer_end - current;
                    match = {(uint16_t) closest, (unsigned char) longest};
                }
                temp++;
            }
            // cout << "\n";
        }
        current--;
    }

    return match;
}

/* Move both buffers steps chars ahead */
int LZ77COMP::moveBuffers(int steps) {
    for(int i = 0; i < steps; i++) {
        // This is only there to not flood the memory
        if(this->history_buffer_end >= this->buffer_max_size){
            this->cleanBuffers();
        }

        // This is to detect EOF
        unsigned char temp = this->input_file.get();
        if ( this->input_file.eof()) {
            // cout << "Reached end of file with i = " << i << "\n";
            // Ensures we never return 0 on EOF
            return i == 0 ? -1 : i;
        }
        // Add new char to end of lookahead buffer, and move char 
        // at lookahead start to history end.
        this->lookahead_buffer.push_back(temp);
        this->history_buffer.push_back(this->lookahead_buffer[this->lookahead_buffer_start]);
        
        this->incrementIndexes();
    }
    return 0;
}

// Supposed to tidy up the vectors, when they reach max size
void LZ77COMP::cleanBuffers() {
    // cout << "Clean buffers!\n";
}

void LZ77COMP::setBufferMaxSize(int size) {
    this->buffer_max_size = size;
}

void LZ77COMP::incrementIndexes() {
    this->lookahead_buffer_start++;
    this->lookahead_buffer_end++;
    this->history_buffer_start++;
    this->history_buffer_end++;
}

void LZ77COMP::pumpHistoryBuffer(int steps) {
    for(int i = 0; i < steps; i++) {
        this->history_buffer.push_back(this->lookahead_buffer[this->lookahead_buffer_start+i]);
    }
}