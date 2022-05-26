#include "RICE.h"
int read_bytes_count = 0;
int written_bytes_count = 0;
int read_blocks_count = 0;

template<typename I>
void RICECODER<I>::encodeSubFrame(I samples_array[], ofstream* output_file, int num_of_samples, int m) {
    this->output_file = output_file;
    // printf("Written bytes count at frame is: %d\n", written_bytes_count);
    printf("First sample: %d; Second sample: %d; Second last sample: %d; Last sample: %d\n", samples_array[0], samples_array[1], samples_array[num_of_samples-2], samples_array[num_of_samples-1]);
    for (int i = 0; i < num_of_samples; i++) {
        // int32_t temp = this->getMappedNumber(samples_array[i]); 
        int32_t temp = static_cast<int32_t>(samples_array[i]);
        // if (i < 20) {
        //     printf("Sample %d is: %d\n", i, temp);
        // }
        this->putRiceEndocedSample(temp, m);
    }
    this->writeBytePadding();
    char space[] = {-1, -1, -1, -1, -1, -1, -1, -1};
    this->output_file->write(space, 8);
}

template<typename I>
void RICECODER<I>::decodeSubFrame(I samples_array[], ifstream* input_file, int num_of_samples, int m) {
    this->input_file = input_file;
    // printf("Read bytes count at frame is: %d\n", read_bytes_count);
    for (int i = 0; i < num_of_samples; i++) {
        samples_array[i] = this->getRiceEncodedSample(m);
        // if (i < 20) {
        //     printf("Sample %d is: %d\n", i, samples_array[i]);
        // }
    }
    printf("First sample: %d; Second sample: %d; Second last sample: %d; Last sample: %d\n", samples_array[0], samples_array[1], samples_array[num_of_samples-2], samples_array[num_of_samples-1]);
    char space[8];
    this->input_file->read(space, 8);
    this->clearReadBuffer();
    // exit(-1);
}

template<typename I>
I RICECODER<I>::getRiceEncodedSample(int m) {
    int32_t temp1 = 0;
    int32_t temp2 = 0;
    // A negative number if first bit is set to 1
    int sign = this->readBit() == 1 ? -1 : 1;
    // Store the first m least significant bits in temp1
    for (int i = 0; i < m; i++) {
        temp1 = (temp1 << 1) | this->readBit();
    }
    while (this->readBit() == 0) {
        temp2++;
    }
    int32_t ret = ((temp2 << m) | temp1)*sign;
    return static_cast<I>(ret);    
}

template<typename I>
void RICECODER<I>::putRiceEndocedSample(int32_t sample, int m) {
    /* First the sign bit is written (1 = -, 0 = +).
           Then the m lowest bits of the positive value of temp are written. The remaining unused bits
           represent the number N. Then N 0's are written, followed by a 1. */

        if (m == 5) {
            printf("Reached weird block!\n");
            printf("Bitbuffer before first sample: %u\n", this->write_bitbuffer);
            printf("Sample: %d\n", sample);
        }
        if (sample < 0) {
            this->writeBit(1);
            sample = sample*-1;
        } else {
            this->writeBit(0);
            if (m==5) {
                printf("0");
            }
        }
        if (m==5) {
            // printf("Sample after possible inversion: %d\n", sample);
            // printf("Written bits after possible inversion: %d\n", this->written_bits);
        }
        for (int i = 0; i < m; i++) {
            // Write first m bits of sample
            if (m==5) {
                printf("%u", (sample & 1));
            }
            this->writeBit(sample & 1);
            sample = sample >> 1;
        }
        if (m==5) {
            // printf("Sample after m bits printed: %d\n", sample);
            // printf("Written bits after m bits printed: %d\n", this->written_bits);
        }
        for (int i = 0; i < sample; i++) {
            this->writeBit(0);
            if (m==5) {
                printf("0");
            }
        }
        this->writeBit(1); 
            if (m==5) {
                printf("1\n");
            }
        if (m == 5) {
            printf("Write bitbuffer after completion of first sample: %u\n", this->write_bitbuffer);
            printf("Written bits after completion of first sample: %d\n", this->written_bits);
            exit(-1);
        }
}

// Codewise the exact reverse of writeBit
template<typename I>
char RICECODER<I>::readBit() {
    if (this->read_bits == 0) {
        this->read_bitbuffer = this->input_file->get();
        // read_bytes_count++;
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