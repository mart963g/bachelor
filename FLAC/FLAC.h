#ifndef FLAC_H
#define FLAC_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>

using namespace std;

class FLACCOMP {
    private:
        int buffer_max_size = 2048;
        vector<unsigned char> buffer;
        ifstream input_file;
        ofstream output_file;
        void initialiseCompression(string file_name);
        void initialiseCompression(string file_name, string destination_file);
        void compressWaveFile();
        void compressOtherFile();
        int pushToBuffer(int n = 1);

    public:
        void compressFile(string file_name);
        void compressFile(string file_name, string destination_file);
        void setBufferMaxSize(int size);
};


#endif