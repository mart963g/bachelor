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
    // if (test == "RIFF") {
    //     this->compressWaveFile();
    // } else {
    //     this->compressOtherFile();
    // }
}

void FLACCOMP::initialiseCompression(string file_name, string destination_file) {
    this->input_file.open(file_name, ios::binary);
    this->pushToBuffer(4);
    string test = (char*) this->buffer.data();
    // if (test == "RIFF") {
    //     this->compressWaveFile();
    // } else {
    //     this->compressOtherFile();
    // }
}

int FLACCOMP::pushToBuffer(int n) {
    for (int i = 0; i < n; i++) {
        // This is to detect EOF
        unsigned char temp = this->input_file.get();
        if ( this->input_file.eof()) {
            // cout << "Reached end of file with i = " << i << "\n";
            // Ensures we never return 0 on EOF
            return i == 0 ? -1 : i;
        }
        this->buffer.push_back(temp);
    }
    return 0;
}