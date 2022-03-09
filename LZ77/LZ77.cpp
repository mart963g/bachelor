#include "LZ77.h"

using namespace std;

void LZ77::openFile(string file_name) {
    this->input_file.open(file_name);

    for(int i = 0; i < this->lookahead_buffer_size; i++){
        this->lookahead_buffer.push_back(this->input_file.get());
    }
    this->lookahead_buffer_index++;
    cout << this->lookahead_buffer.data();

    // for(int i = 0; i < this->lookahead_buffer_size; i++){
    //     cout << this->lookahead_buffer[i];
    // }
    cout << "\n";

    // cout << "Hello World!\n" << file_name << "\n";
}

// NOT DONE
/* Move both buffers one char ahead */
void LZ77::moveBuffers() {
    if(this->lookahead_buffer_index == this->lookahead_buffer_size || this->history_buffer_index == this->history_buffer_size){
        this->cleanBuffers();
    }
    // Add char at lookahead buffer index in lookahead buffer, to end of history buffer
    // Increase both indexes by 1
    this->lookahead_buffer.push_back(this->input_file.get());
    this->history_buffer.push_back(this->lookahead_buffer.data()[this->lookahead_buffer_index++]);
    this->history_buffer_index++;
}

void LZ77::cleanBuffers() {
    cout << "Clean buffers!\n";
}
