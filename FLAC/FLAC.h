#ifndef FLAC_H
#define FLAC_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>

using namespace std;

struct waveHeader {
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
};

class FLACCOMP {
    private:
        int buffer_max_size = 2048;
        vector<unsigned char> buffer;
        ifstream input_file;
        ofstream output_file;
        int buffer_start = 0;
        int buffer_end = 0;
        void initialiseCompression(string file_name);
        void initialiseCompression(string file_name, string destination_file);
        void compressWaveFile();
        void compressOtherFile();
        int pushToBuffer(int n = 1);
        void cleanBuffer();

    public:
        void compressFile(string file_name);
        void compressFile(string file_name, string destination_file);
        void setBufferMaxSize(int size);
};


#endif