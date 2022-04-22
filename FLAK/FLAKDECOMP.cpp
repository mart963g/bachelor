#include "FLAK.h"

using namespace std;

struct DataFrame<int16_t> frame;

void FLAKDECOMP::decompressFile(string file_name) {
    this->initialiseDecompression(file_name, file_name);
    this->input_file.close();
    this->output_file.close();
}

void FLAKDECOMP::decompressFile(string file_name, string destination_file) {
    this->initialiseDecompression(file_name, destination_file);
    this->input_file.close();
    this->output_file.close();
}


void FLAKDECOMP::initialiseDecompression(string file_name, string destination_file) {
    this->input_file.open(file_name, ios::binary);
    this->output_file.open(destination_file, ios::binary);
    this->pushToBuffer(4);
    string flak = (char*) this->buffer.data();
    if (flak != "FLAK") {
        printf("ERROR: This is not a compressed FLAK file!\n");
        return;
    }
    this->pushToBuffer(4);
    string test = (char*) &this->buffer.data()[4];
    if (test == "RIFF") {
        this->pushToBuffer(8);
        string wave = (char*) &this->buffer.data()[12];
        if (wave == "WAVE") {
            this->decompressWaveFile();
        } else {
            this->decompressOtherFile();
        }
    } else {
        this->decompressOtherFile();
    }
}

void FLAKDECOMP::decompressWaveFile() {
    printf("This was a wave file!\n");
}

void FLAKDECOMP::decompressOtherFile() {
    printf("This was not a wave file!\n");
}

int FLAKDECOMP::pushToBuffer(int n) {
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

void FLAKDECOMP::cleanBuffer() {
    // Whoopsie :-/
}