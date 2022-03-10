#include "LZ77.h"

using namespace std;

void LZ77::openFile(string file_name) {
    this->input_file.open(file_name);
    this->initialiseBuffers();
    
    // this->lookahead_buffer_index++;
    cout << this->lookahead_buffer.data();

    // for(int i = 0; i < this->lookahead_buffer_size; i++){
    //     cout << this->lookahead_buffer[i];
    // }
    cout << "\n";

    // cout << "Hello World!\n" << file_name << "\n";
}

void LZ77::initialiseBuffers(int history_size, int lookahead_size) {
    this->lookahead_buffer_end = lookahead_size;
    for(int i = this->lookahead_buffer_start; i < this->lookahead_buffer_end; i++){
        this->lookahead_buffer.push_back(this->input_file.get());
    }
    // History buffer is initialised to be empty
    this->history_buffer_end = 0;
    /* Insert some code that runs until history buffer is full */
    while(this->history_buffer_end < history_size - 1) {
        // Do something and then
        this->history_buffer_end++;
    }
}

/* Move both buffers steps chars ahead */
void LZ77::moveBuffers(int steps) {
    for(int i = 0; i < steps; i++) {
        if(this->history_buffer_end >= this->buffer_max_size){
            this->cleanBuffers();
        }
        // Add new char to end of lookahead buffer, and move char 
        // at lookahead start to history end.
        this->lookahead_buffer.push_back(this->input_file.get());
        this->history_buffer.push_back(this->lookahead_buffer.data()[this->lookahead_buffer_start]);
        
        this->incrementIndexes();
    }
}

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
