#ifndef FLAC_H
#define FLAC_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <netinet/in.h>

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
        struct waveHeader header;
        int buffer_max_size = 2048;
        vector<unsigned char> buffer;
        ifstream input_file;
        ofstream output_file;
        int buffer_end = 0;
        void initialiseCompression(string file_name);
        void initialiseCompression(string file_name, string destination_file);
        void compressWaveFile();
        void compressOtherFile();
        int pushToBuffer(int n = 1);
        int fillOutHeader();
        void cleanBuffer();
        uint16_t getShortFromLittleEndianBuffer(int start_index);
        uint32_t getLongFromLittleEndianBuffer(int start_index);

    public:
        void compressFile(string file_name);
        void compressFile(string file_name, string destination_file);
        void setBufferMaxSize(int size);
};


#endif