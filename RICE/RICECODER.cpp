#include "RICE.h"

template<typename I>
void RICECODER<I>::encodeFrame(I samples_array[], ofstream output_file, int num_of_samples, int m) {
    this->output_file = output_file;
    for (int i = 0; i < num_of_samples; i++) {
        int32_t temp = this->getMappedNumber(samples_array[i]);
        // for i=0 < m 
        //     WRITE temp | 1
        //     temp = temp >> 1
        // for i=0 < temp
        //     WRITE 0
        // WRITE 1      
        /* First the m lowest bits of temp are written. The remaining unused bits
           represent the number N. Then N 0's are written, followed by a 1. */
        for (size_t i = 0; i < m; i++) {
            this->writeBit(temp | 1);
            temp = temp >> 1;
        }
        for (size_t i = 0; i < temp; i++) {
            this->writeBit(0);
        }
        this->writeBit(1);        
    }
    this->writeBytePadding();
}

template<typename I>
void RICECODER<I>::writeBit(char bit) {
    this->write_bitbuffer = this->write_bitbuffer << 1 | bit
    this->written_bits++;
    if (this->written_bits == 8) {
        this->output_file.write(this->write_bitbuffer);
        this->write_bitbuffer = 0
        this->written_bits = 0
    }
}

template<typename I>
void RICECODER<I>::writeBytePadding() {
    if (this->written_bits > 0) {
        // Pads the remaning bits with 0's, to reach a full byte, and the writes it
        // to the output file. Should be done for every subframe, to start 
        // the new frame on a real byte, for ease of implementation.
        this->write_bitbuffer = (this->write_bitbuffer << (8-this->written_bits));
        this->output_file.write(this->write_bitbuffer);
    }
}

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