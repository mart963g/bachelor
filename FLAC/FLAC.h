#ifndef FLAC_H
#define FLAC_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <netinet/in.h>

using namespace std;

// Defines the frame size used in the program
const int frame_sample_size_const = 576;

struct waveHeader {
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
};

// Generic to maybe support different bit depths in future
template<typename I>
struct DataFrame {
    // Supporting stereo files as the default
    I left[frame_sample_size_const];
    I right[frame_sample_size_const];
    I e0[frame_sample_size_const];
    I e1[frame_sample_size_const];
    I e2[frame_sample_size_const];
    I e3[frame_sample_size_const];
    long e0_sum = 0;
    long e1_sum = 0;
    long e2_sum = 0;
    long e3_sum = 0;
};

class FLACCOMP {
    private:
        struct waveHeader wave_header;
        int buffer_max_size = 2048;
        int frame_sample_size = frame_sample_size_const;
        int sample_byte_depth = 2;
        vector<unsigned char> buffer;
        ifstream input_file;
        ofstream output_file;
        int buffer_end = 0;
        void initialiseCompression(string file_name, string destination_file);
        void compressWaveFile();
        void compressOtherFile();
        int pushToBuffer(int n = 1);
        int fillOutHeader();
        int fillOutFrame();
        void processFrame();
        void initialiseErrorArrays(string channel);
        int writeFrame();
        void cleanBuffer();
        uint16_t getShortFromLittleEndianBuffer(int start_index);
        uint32_t getLongFromLittleEndianBuffer(int start_index);
        int16_t getSignedShortFromLittleEndianBuffer(int start_index);

    public:
        void compressFile(string file_name);
        void compressFile(string file_name, string destination_file);
        void setFrameSampleSize(int size);
        void setBufferMaxSize(int size);
};


#endif