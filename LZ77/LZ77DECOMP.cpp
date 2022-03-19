#include "LZ77.h"

using namespace std;

void LZ77DECOMP::setBufferMaxSize(int size) {
    this->buffer_max_size = size;
}

void LZ77DECOMP::initialiseDecompression(string file_name) {
    this->input_file.open(file_name, ios::binary);
    auto pos = file_name.find(".lzip");
    this->output_file.open(file_name.substr(0, pos), ios::binary);
}

void LZ77DECOMP::initialiseDecompression(string file_name, string destination_file) {
    this->input_file.open(file_name, ios::binary);
    this->output_file.open(destination_file, ios::binary);
}

void LZ77DECOMP::decompressFile(string file_name) {
    this->initialiseDecompression(file_name);
    this->decompressAfterInitialisation();
    this->input_file.close();
    this->output_file.close();
}

void LZ77DECOMP::decompressFile(string file_name, string destination_file) {
    this->initialiseDecompression(file_name, destination_file);
    this->decompressAfterInitialisation();
    this->input_file.close();
    this->output_file.close();
}

void LZ77DECOMP::decompressAfterInitialisation() {
    struct token token;
    while (readTokenFromInput(token) == 0) {
        this->writeBytesFromToken(token);
    }
}

int LZ77DECOMP::readTokenFromInput(struct token &token) {
    token.offset = this->input_file.get();
    token.length = this->input_file.get();
    return this->input_file.eof() ? 1 : 0;
}

void LZ77DECOMP::moveBuffer(int steps) {
    if (this->buffer_end + steps > this->buffer_max_size) {
        this->cleanBuffer();
    }
    for(int i = 0; i < steps; i++) {
        this->output_file.put(this->buffer[this->buffer_end]);
        this->buffer_end++;
    }
}

void LZ77DECOMP::writeBytesFromToken(struct token token) {
    // If the token does not represent a match simply but the data in the buffer and files
    if (token.offset == 0) {
        this->buffer.push_back(token.length);
        this->moveBuffer();
    } // Otherwise loop over the elements from the offset 
    else {
        int offset = (int) token.offset;
        for (int i = 0; i < token.length; i++) {
            // This works because the buffer_end, gets incremented in every iteration
            this->buffer.push_back(this->buffer[this->buffer_end - offset]);
            this->moveBuffer();
        }   
    }
}

void LZ77DECOMP::cleanBuffer() {
    cout << "Clean buffer!\n";
}