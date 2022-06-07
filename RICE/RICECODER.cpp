#include "RICE.h"
#include <cstring>
int read_bytes_count = 0;
int written_bytes_count = 0;
int read_blocks_count = 0;
int written_blocks_count = 0;

template<typename I>
void RICECODER<I>::encodeSubFrame(I samples_array[], int num_of_samples, int m) {
    for (int i = 0; i < num_of_samples; i++) {
        int32_t temp = static_cast<int32_t>(samples_array[i]);
        this->putRiceEncodedSample(temp, m);
    }
    this->writeBytePadding();
}

template<typename I>
void RICECODER<I>::decodeSubFrame(I samples_array[], int num_of_samples, int m) {
    for (int i = 0; i < num_of_samples; i++) {
        samples_array[i] = this->getRiceEncodedSample(m);
    }
    this->clearReadBuffer();
}

template<typename I>
I RICECODER<I>::getRiceEncodedSample(int m) {
    int32_t temp1 = 0;
    int32_t temp2 = 0;
    // A negative number if first bit is set to 1
    int sign = this->readBit() == 1 ? -1 : 1;
    // Store the first m least significant bits in temp1
    for (int i = 0; i < m; i++) {
        temp1 = temp1 | (this->readBit() << i);
    }
    while (this->readBit() == 0) {
        temp2++;
    }
    int32_t ret = ((temp2 << m) | temp1)*sign;
    return static_cast<I>(ret);    
}

template<typename I>
void RICECODER<I>::putRiceEncodedSample(int32_t sample, int m) {
    /*  First the sign bit is written (1 = -, 0 = +).
        Then the m lowest bits of the positive value of temp are written. The remaining unused bits
        represent the number N. Then N 0's are written, followed by a 1. */
    if (sample < 0) {
        this->writeBit(1);
        sample = sample*-1;
    } else {
        this->writeBit(0);
    }
    for (int i = 0; i < m; i++) {
        // Write first m bits of sample
        this->writeBit(sample & 1);
        sample = sample >> 1;
    }
    for (int i = 0; i < sample; i++) {
        this->writeBit(0);
    }
    this->writeBit(1); 
}

// Codewise the exact reverse of writeBit
template<typename I>
char RICECODER<I>::readBit() {
    if (this->read_bits == 0) {
        this->read_bitbuffer = this->input_file->get();
        read_bytes_count++;
        this->read_bits = 8;
    }
    this->read_bits--;
    char bit = (this->read_bitbuffer & 128) >> 7;
    this->read_bitbuffer = this->read_bitbuffer << 1;
    return bit; 
}

template<typename I>
void RICECODER<I>::writeBit(char bit) {
    this->write_bitbuffer = this->write_bitbuffer << 1 | bit;
    this->written_bits++;
    if (this->written_bits == 8) {
        this->output_file->put(this->write_bitbuffer);
        written_bytes_count++;
        this->write_bitbuffer = 0;
        this->written_bits = 0;
    }
}

template<typename I>
void RICECODER<I>::writeBytePadding() {
    if (this->written_bits > 0) {
        // Pads the remaning bits with 0's, to reach a full byte, and then writes it
        // to the output file. Should be done for every subframe, to start 
        // the new frame on a real byte, for ease of implementation.
        this->write_bitbuffer = (this->write_bitbuffer << (8-this->written_bits));
        this->output_file->put(this->write_bitbuffer);
        this->write_bitbuffer = 0;
        this->written_bits = 0;
    }
}

template<typename I>
void RICECODER<I>::clearReadBuffer() {
    this->read_bitbuffer = 0;
    this->read_bits = 0;
}

template<typename I>
void RICECODER<I>::setOutputFile(ofstream* output_file) {
    this->output_file = output_file;
}

template<typename I>
void RICECODER<I>::setInputFile(ifstream* input_file) {
    this->input_file = input_file;
}

// These two following functions, are only used in the case of Golomb coding,
// which was found to perform worse than Rice coding.
template<>
int32_t RICECODER<int16_t>::getMappedNumber(int16_t number) {
    int32_t temp = number;
    // This is a mapping from signed integers, to unsigned integers, since RICE
    // coding doesn't work for negative numbers
    temp = temp < 0 ? (-2*temp - 1) : (2*temp);
    return temp;
}


template<>
int32_t RICECODER<unsigned char>::getMappedNumber(unsigned char number) {
    int32_t temp = number;
    return temp;
} 

// Declares which instantiations the compiler needs to make, 
// for the linker to be able to reference them.
template class RICECODER<int16_t>;
// template class RICECODER<unsigned char>;