#ifndef FLAC_H
#define FLAC_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <string.h>
#include <netinet/in.h>

using namespace std;

// Defines the frame size used in the program
const int frame_sample_size_const = 576;

struct waveHeader {
    uint32_t FileSize;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    uint32_t DataSize;
};

// Generic to maybe support different bit depths in future
template<typename I>
struct DataFrame {
    // Supporting stereo files as the default
    I left[frame_sample_size_const];
    I right[frame_sample_size_const];
};

// Generic to maybe support different bit depths in future
template<typename I>
struct ErrorWrapper {
    I e0[frame_sample_size_const];
    I e1[frame_sample_size_const];
    I e2[frame_sample_size_const];
    I e3[frame_sample_size_const];
    long sums[4];
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
        void processSubFrame(string channel);
        void initialiseErrorArrays(string channel);
        void processErrors(string channel);
        void encodeResiduals(int order);
        int writeSubFrame(string channel);
        void writeSubFrameRaw(string channel, int order, int samples = frame_sample_size_const);
        void writeSignedShortToFile(int16_t number);
        void cleanBuffer();
        void writeWaveHeader();
        void processLastFrame(int samples);
        void processLastSubFrame(string channel, int samples);
        uint16_t getShortFromLittleEndianBuffer(int start_index);
        uint32_t getLongFromLittleEndianBuffer(int start_index);
        int16_t getSignedShortFromLittleEndianBuffer(int start_index);

    public:
        void compressFile(string file_name);
        void compressFile(string file_name, string destination_file);
        void setFrameSampleSize(int size);
        void setBufferMaxSize(int size);
};

class FLACDECOMP {
    private:
        struct waveHeader wave_header;
        ifstream input_file;
        ofstream output_file;
        vector<unsigned char> buffer;
        int buffer_max_size = 2048;
        int buffer_end = 0;
        int fillOutHeader();
        void initialiseDecompression(string file_name, string destination_file);
        int pushToBuffer(int n = 1);

    public:
        void decompressFile(string file_name);
        void decompressFile(string file_name, string destination_file);
        void setBufferMaxSize(int size);
};


#endif