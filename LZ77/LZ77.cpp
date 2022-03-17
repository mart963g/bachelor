#include "LZ77.h"

using namespace std;

void LZ77::compressFile(string file_name) {
    this->initialiseCompression(file_name);
    this->compressAfterInitialisation();
    this->input_file.close();
    this->output_file.close();
}

void LZ77::initialiseCompression(string file_name) {
    // Open a name with filename.zip
    this->input_file.open(file_name, ios::binary);
    this->output_file.open(file_name + ".lzip", ios::app | ios::binary);

    this->initialiseBuffers();
}

/* Has defaults history_size = 10, lookahead_size = 5 */
void LZ77::initialiseBuffers(int history_size, int lookahead_size) {
    this->lookahead_buffer_end = lookahead_size;
    for(int i = this->lookahead_buffer_start; i < this->lookahead_buffer_end; i++){
        this->lookahead_buffer.push_back(this->input_file.get());
    }
    /* Runs until history buffer is full */
    while(this->history_buffer_end < history_size) {
        // cout << "----------------------------------\n";
        struct token token = this->searchForMatch();
        this->writeTokenToOutputFile(token);
        // printf("Offset: %u; Length: %u\n", token.offset, token.length);

        // If no match was found, move 1 char, otherwise move the length of the match
        int move = (token.offset == 0 ? 1 : (int) token.length);
        // cout << "Move: " << move << "\n";

        // NEED AN EXTRA CHECK HERE TO PROTECT AGAINST EOF!!!!
        this->moveBuffers(move);
        // NEED AN EXTRA CHECK HERE TO PROTECT AGAINST EOF!!!!

        // If this is the last run, set start to preserve history buffer size otherwise keep start at 0
        this->history_buffer_start = this->history_buffer_end >= history_size ? history_buffer_end - history_size : 0;
        // printf("History start: %d; History end: %d\n", this->history_buffer_start, this->history_buffer_end);
        // printf("Lookahead start: %d; Lookahead end: %d \n", this->lookahead_buffer_start, this->lookahead_buffer_end);
    }
}

void LZ77::compressAfterInitialisation() {
    int move = 0;
    struct token token; 
    int remainder = 0;
    while ( (remainder = this->moveBuffers(move)) == 0) {
        token = this->searchForMatch();
        this->writeTokenToOutputFile(token);

        // If no match was found, move 1 char, otherwise move the length of the match
        move = (token.offset == 0 ? 1 : (int) token.length);
    }
    cout << "Remainder: " << remainder << "\n";
    cout << "Move: " << move << "\n";
    // Finish the moving of the buffers, without changing then end of the lookahead buffer
    if (remainder != -1) {
        this->lookahead_buffer_start += (move - remainder);
        this->history_buffer_start += (move - remainder);
        this->history_buffer_end += (move - remainder);
    }
    // Deal with the remaining bytes of the file
    while (this->lookahead_buffer_start < this->lookahead_buffer_end) {
        token = this->searchForMatch();
        this->writeTokenToOutputFile(token);

        // If no match was found, move 1 char, otherwise move the length of the match
        move = (token.offset == 0 ? 1 : (int) token.length);
        this->lookahead_buffer_start += move;
        this->history_buffer_start += move;
        this->history_buffer_end += move;
    }

}

void LZ77::writeTokenToOutputFile(struct token token) {
    this->output_file.put(token.offset);
    this->output_file.put(token.length);
}

/* 
Start with first character in lookahead buffer, search backwards in history buffer. If there
is a match check if it is also a match for next char in lookahead buffer. If this is not the
 case, save placement and length somewhere, and look in the rest,of the history buffer. 
When the history buffer start is reached, pick the longest closest match, and return it.    */

/* Finds and returns the longest match, closest to the lookahead buffer */
struct token LZ77::searchForMatch() {
    int longest = 1; // We only want matches of length >= 2
    int closest = 0; // This default value does not matter
    
    // Small optimisation, so a pointer is not accessed everytime
    unsigned char first = this->lookahead_buffer[this->lookahead_buffer_start];
    
    // The default return value, if no match is found
    struct token match = {0, first};

    int current = this->history_buffer_end - 1;
    // Go from right to left
    while(current >= this->history_buffer_start) {
        // If a match is found, go from left to rigth
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
                    match = {(unsigned char) closest, (unsigned char) longest};
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
int LZ77::moveBuffers(int steps) {
    for(int i = 0; i < steps; i++) {
        // This is only there to not flood the memory
        if(this->history_buffer_end >= this->buffer_max_size){
            this->cleanBuffers();
        }

        // This is to detect EOF
        unsigned char temp = this->input_file.get();
        if ( this->input_file.eof()) {
            cout << "Reached end of file with i = " << i << "\n";
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
void LZ77::cleanBuffers() {
    cout << "Clean buffers!\n";
}

void LZ77::setBufferMaxSize(int size) {
    this->buffer_max_size = size;
}

void LZ77::incrementIndexes() {
    this->lookahead_buffer_start++;
    this->lookahead_buffer_end++;
    this->history_buffer_start++;
    this->history_buffer_end++;
}
