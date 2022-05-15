#ifndef RICE_H
#define RICE_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;


template<typename I>
class RICECODER {
    private:
        ofstream* output_file;
        ifstream input_file;
        unsigned char write_bitbuffer = 0;
        unsigned char read_bitbuffer = 0;
        int written_bits = 0;
        int read_bits = 0;
        int32_t getMappedNumber(I number);
        void writeBytePadding();
        void writeBit(char bit);
    
    public:
        void encodeSubFrame(I samples_array[], ofstream* output_file, int num_of_samples, int m);
        void decodeSubFrame(I samples_array[], ifstream* input_file, int num_of_samples, int m);
};

#endif