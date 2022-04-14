#include "FLAC.h"

using namespace std;

void FLACCOMP::compressFile(string file_name) {
    this->initialiseCompression(file_name);
    this->input_file.close();
    this->output_file.close();
}

void FLACCOMP::compressFile(string file_name, string destination_file) {
    this->initialiseCompression(file_name, destination_file);
    this->input_file.close();
    this->output_file.close();
}

void FLACCOMP::initialiseCompression(string file_name) {
    this->input_file.open(file_name, ios::binary);
    this->pushToBuffer(4);   
    string test = (char*) this->buffer.data();
    if (test == "RIFF") {
        this->pushToBuffer(8);
        string wave = (char*) &this->buffer.data()[8];
        if (wave == "WAVE") {
            this->compressWaveFile();
        } else {
            this->compressOtherFile();
        }
    } else {
        this->compressOtherFile();
    }
}

void FLACCOMP::initialiseCompression(string file_name, string destination_file) {
    this->input_file.open(file_name, ios::binary);
    this->pushToBuffer(4);
    string test = (char*) this->buffer.data();
    if (test == "RIFF") {
        this->pushToBuffer(8);
        string wave = (char*) &this->buffer.data()[8];
        if (wave == "WAVE") {
            this->compressWaveFile();
        } else {
            this->compressOtherFile();
        }
    } else {
        this->compressOtherFile();
    }
}

void FLACCOMP::compressWaveFile() {
    printf("This is a wave file!\n");
}

void FLACCOMP::compressOtherFile() {
    printf("This is not a wave file!\n");
}

int FLACCOMP::pushToBuffer(int n) {
    for (int i = 0; i < n; i++) {
        if(this->buffer_end >= this->buffer_max_size){
            this->cleanBuffer();
        }

        // This is to detect EOF
        unsigned char temp = this->input_file.get();
        if ( this->input_file.eof()) {
            // cout << "Reached end of file with i = " << i << "\n";
            // Ensures we never return 0 on EOF
            return i == 0 ? -1 : i;
        }
        this->buffer.push_back(temp);
        this->buffer_end++;
    }
    return 0;
}

void FLACCOMP::cleanBuffer() {
    // Hopefully at some point this will be implemented
}