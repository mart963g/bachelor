#include "FLAK.h"

using namespace std;

struct DataFrame<int16_t> read_frame;
struct DataFrame<int16_t> write_frame;
struct ErrorWrapper<int16_t> error_struct;

// JUST FOR TESTING!!
int global_count = 0;

// This global integer is used to keep track of,
// if there are more samples in the left channel
// than in the right in the case of stereo sound
int decompression_different_sample_number_in_channels_flag = 0;

void FLAKDECOMP::decompressFile(string file_name) {
    this->initialiseDecompression(file_name, file_name);
    this->input_file.close();
    this->output_file.close();
}

void FLAKDECOMP::decompressFile(string file_name, string destination_file) {
    this->initialiseDecompression(file_name, destination_file);
    this->input_file.close();
    this->output_file.close();
}


void FLAKDECOMP::initialiseDecompression(string file_name, string destination_file) {
    this->input_file.open(file_name, ios::binary);
    this->output_file.open(destination_file, ios::binary);
    this->pushToBuffer(4);
    if (memcmp(this->buffer.data(), "FLAK", 4)) {
        printf("ERROR: This is not a compressed FLAK file!\n");
        return;
    }
    this->pushToBuffer(4);
    char* test = (char*) &this->buffer.data()[4];
    if (!memcmp(test, "RIFF", 4)) {
        this->pushToBuffer(8);
        char* wave = (char*) &this->buffer.data()[12];
        if (!memcmp(wave, "WAVE", 4)) {
            this->decompressWaveFile();
        } else {
            this->decompressOtherFile();
        }
    } else {
        this->decompressOtherFile();
    }
}

void FLAKDECOMP::decompressWaveFile() {
    // printf("This was a wave file!\n");
    this->fillOutHeader();
    this->writeWaveHeader();
    int computed_samples = 0;
    while((computed_samples = this->readFrame()) == 0) {
        this->writeFrame();
    }
}

void FLAKDECOMP::decompressOtherFile() {
    // printf("This was not a wave file!\n");
    int computed_samples = this->readFirstFrameNonWaveFile(this->buffer.size());
    this->writeFrame();
    while((computed_samples = this->readFrame()) == 0) {
        this->writeFrame();
    }
}

int FLAKDECOMP::fillOutHeader() {
    this->wave_header.FileSize = this->getLongFromLittleEndianBuffer(8);
    this->pushToBuffer(10);
    this->wave_header.AudioFormat = this->getShortFromLittleEndianBuffer(this->buffer_end-2);
    this->pushToBuffer(2);
    this->wave_header.NumChannels = this->getShortFromLittleEndianBuffer(this->buffer_end-2);
    this->pushToBuffer(4);
    this->wave_header.SampleRate = this->getLongFromLittleEndianBuffer(this->buffer_end-4);
    this->pushToBuffer(4);
    this->wave_header.ByteRate = this->getLongFromLittleEndianBuffer(this->buffer_end-4);
    this->pushToBuffer(2);
    this->wave_header.BlockAlign = this->getShortFromLittleEndianBuffer(this->buffer_end-2);
    this->pushToBuffer(2);
    this->wave_header.BitsPerSample = this->getShortFromLittleEndianBuffer(this->buffer_end-2);
    this->sample_byte_depth = this->wave_header.BitsPerSample/8;
    this->pushToBuffer(4);
    char* data = (char*) &this->buffer.data()[this->buffer_end-4];
    if (!memcmp(data, "data", 4)) {
        this->pushToBuffer(4);
        this->wave_header.DataSize = this->getLongFromLittleEndianBuffer(this->buffer_end-4);
    } else {
        printf("ERROR: Something is wrong with the header data!\n");
        // return -1;
    }
    // printf("Audioformat:        %u\n", this->wave_header.AudioFormat);
    // printf("Number of channels: %u\n", this->wave_header.NumChannels);
    // printf("Sample rate:        %u\n", this->wave_header.SampleRate);
    // printf("Byterate:           %u\n", this->wave_header.ByteRate);
    // printf("Block align:        %u\n", this->wave_header.BlockAlign);
    // printf("Bits per sample:    %u\n", this->wave_header.BitsPerSample);
    return this->input_file.eof();
}

void FLAKDECOMP::writeWaveHeader() {
    this->output_file.write((char*) &this->buffer.data()[4], 44);
}

int FLAKDECOMP::readFrame() {
    int ret = this->readSubFrame();
    if (ret != 0) return ret;

    if (this->wave_header.NumChannels > 1) {
        ret = this->readSubFrame();
    }

    return ret;
}

int FLAKDECOMP::readSubFrame() {
    if (this->pushToBuffer(1) != 0) {
        return -1;
    }
    unsigned char header_char = this->buffer[this->buffer_end - 1]; 
    unsigned char write_channel = (header_char & 64) >> 6;
    unsigned char last_subframe_flag = (header_char & 128) >> 7;
    unsigned char write_order = (header_char >> 4) & 3;
    unsigned char m = header_char & 15;
    string channel = write_channel == 0 ? "left" : "right";
    int sample_limit = this->frame_sample_size;
    // if (global_count > 24) {
        printf("Header char: %u\t\tFlag: %u\t\tOrder: %u\t\tChannel: %s\t\tM: %u\n", header_char, last_subframe_flag, write_order, (write_channel == 0 ? "left" : "right"), m);
    // }
    if (last_subframe_flag) {
        this->pushToBuffer(2);
        sample_limit = this->getSignedShortFromLittleEndianBuffer(this->buffer_end-2);
        // This is not pretty, but it is the fastest way to make writeFrame correct
        if (sample_limit != this->frame_sample_size && this->frame_sample_size != frame_sample_size_const) {
            decompression_different_sample_number_in_channels_flag = 1;
        }
        this->frame_sample_size = sample_limit;
        // printf("Samples in last frame: %d\n", sample_limit);
    }
    // cout << "Channel: " + channel;
    // printf("\tOrder: %u\tK: %u\n", write_order, k);
    int ret = 0;
    // int count = 0;
    for (int i = 0; i < write_order; i++) {
    // for (int i = 0; i < 2; i++) {
        ret = this->pushToBuffer(this->sample_byte_depth);
        if (ret != 0) {
            return (i == 0 ? -1 : i);
        }
        if (write_channel == 0) {
            if (this->sample_byte_depth > 1) {
                // This function makes some assumptions, that breaks if the bit depth is not 16
                read_frame.left[i] = this->getSignedShortFromLittleEndianBuffer(this->buffer_end - this->sample_byte_depth);
            } else {
                read_frame.left[i] = this->buffer[this->buffer_end-1];
            }
        } else {
            if (this->sample_byte_depth > 1) {
                read_frame.right[i] = this->getSignedShortFromLittleEndianBuffer(this->buffer_end - this->sample_byte_depth);
            } else {
                read_frame.right[i] = this->buffer[this->buffer_end-1];
            }
        }
    }
    if (write_channel == 0) {
        this->rice_16.decodeSubFrame(read_frame.left + write_order, &this->input_file, sample_limit-write_order, m);
    } else {
        this->rice_16.decodeSubFrame(read_frame.right + write_order, &this->input_file, sample_limit-write_order, m);
    }
    this->processSubFrame(channel, write_order, sample_limit);
    return ret;
}

void FLAKDECOMP::processSubFrame(string channel, int order, int samples) {
    if (channel == "left") {
        for (int i = 0; i < order; i++) {
            write_frame.left[i] = read_frame.left[i];
            // if (this->buffer.size() < 1000) {
            //     printf("DECOMP: Sample number %d in last subframe is: %d\n", i, write_frame.left[i]);
            //     printf("DECOMP: Real value is (cheating): %d\n", write_frame.left[i]);
            // }
        }
        
    } else {
        for (int i = 0; i < order; i++) {
           write_frame.right[i] = read_frame.right[i];
            // if (samples != this->frame_sample_size) {
            //     printf("DECOMP: Sample number %d in last subframe is: %d\n", i, write_frame.right[i]);
            //     printf("DECOMP: Real value is (cheating): %d\n", write_frame.right[i]);
            // }
        }
    }
    for (int i = order; i < samples; i++) {
        // Read residual, and store previous real values in placeholders for the prediction.
        // This is done to avoid if statements with regards to the channel, inside the switch.
        int16_t cur_error = 0;
        int16_t result_1 = 0;
        int16_t result_2 = 0;
        int16_t result_3 = 0;
        if (channel == "left") {
            cur_error = read_frame.left[i];
            if (order > 0) result_1 = write_frame.left[i-1];
            if (order > 1) result_2 = write_frame.left[i-2];
            if (order > 2) result_3 = write_frame.left[i-3];
        } else {
            cur_error = read_frame.right[i];
            if (order > 0) result_1 = write_frame.right[i-1];
            if (order > 1) result_2 = write_frame.right[i-2];
            if (order > 2) result_3 = write_frame.right[i-3];
        }
        // This is an int, to avoid overflow in the calculations
        // Might actually cause a problem in the conversion, we will see
        int real_result = 0;
        switch (order) {
        case 0:
            real_result = cur_error;
            break;
        
        case 1:
            real_result = result_1 + cur_error;
            break;
        
        case 2:
            real_result = 2*result_1 - result_2 + cur_error;
            break;

        case 3:
            real_result = 3*result_1 - 3*result_2 + result_3 + cur_error;
            break;
        
        default:
            printf("ERROR: Invalid order used in processing, order: %d\n", order);
        }
        if (channel == "left") {
            write_frame.left[i] = (int16_t) real_result;
        } else {
            write_frame.right[i] = (int16_t) real_result;
        }
    }
    
}

void FLAKDECOMP::writeFrame() {
    for (int i = 0; i < this->frame_sample_size; i++) {
        if (this->sample_byte_depth > 1) {
            this->writeSignedShortToFile(write_frame.left[i]);
        } else {
            this->output_file.put((unsigned char) write_frame.left[i]);
        }
        if (this->wave_header.NumChannels > 1) {
            if (this->sample_byte_depth > 1) {
                this->writeSignedShortToFile(write_frame.right[i]);
            } else {
                this->output_file.put((unsigned char) write_frame.right[i]);
            }
        }
    }
    // If there were one more sample in the left channel, write that as well
    if (this->frame_sample_size != frame_sample_size_const && decompression_different_sample_number_in_channels_flag) {
        if (this->sample_byte_depth > 1) {
            this->writeSignedShortToFile(write_frame.left[this->frame_sample_size]);
        } else {
            this->output_file.put((unsigned char) write_frame.left[this->frame_sample_size]);
        }
    }
}

int FLAKDECOMP::readFirstFrameNonWaveFile(int buffer_size) {
    unsigned char header_char = this->buffer[4]; 
    unsigned char write_channel = (header_char & 64) >> 6;
    unsigned char last_subframe_flag = (header_char & 128) >> 7;
    unsigned char write_order = (header_char >> 4) & 3;

    int start = 5;
    string channel = write_channel == 0 ? "left" : "right";
    int sample_limit = this->frame_sample_size;
    // printf("Header char: %u\t\tFlag: %u\n", header_char, last_subframe_flag);
    if (last_subframe_flag) {
        start = 7;
        sample_limit = this->getSignedShortFromLittleEndianBuffer(5);
        // This is not pretty, but it is the fastest way to make writeFrame correct
        if (sample_limit != this->frame_sample_size && this->frame_sample_size != frame_sample_size_const) {
            decompression_different_sample_number_in_channels_flag = 1;
        }
        this->frame_sample_size = sample_limit;
        // printf("Samples in last frame: %d\n", sample_limit);
    }

    int ret = 0;
    for (int i = start; i < (sample_limit + start); i++) {
        // Only adds to the buffer if the current data is not yet read.
        // This line is the whole reason for the separate function.
        if (i >= buffer_size) {
            ret = this->pushToBuffer(this->sample_byte_depth);
            if (ret != 0) {
                return (i == 0 ? -1 : i);
            }
            // Non wave files are always encoded with mono sound 8-bits for simplicity.
            read_frame.left[i-start] = this->buffer[this->buffer_end-1];
        } else {
            read_frame.left[i-start] = this->buffer[i];
        }
    }
    this->processSubFrame(channel, write_order, sample_limit);

    return ret;
}

void FLAKDECOMP::writeSignedShortToFile(int16_t number) {
    unsigned char write = number & 255;
    // printf("First char: %u\n", write);
    this->output_file.put(write);
    write = (number >> 8) & 255;
    // printf("Second char: %u\n", write);
    this->output_file.put(write);
}

int FLAKDECOMP::pushToBuffer(int n) {
    for (int i = 0; i < n; i++) {
        if(this->buffer_end >= this->buffer_max_size){
            this->cleanBuffer();
        }
        // This is to detect EOF
        unsigned char temp = this->input_file.get();
        if ( this->input_file.eof()) {
            // cout << "Reached end of file with i = " << i << "\n";
            // Ensures we never return 0 on EOF
            return i == 0 ? -1 : i;
        }
        this->buffer.push_back(temp);
        this->buffer_end++;
    }
    return 0;
}

void FLAKDECOMP::cleanBuffer() {
    // Whoopsie :-/
}

void FLAKDECOMP::setBufferMaxSize(int size) {
    this->buffer_max_size = size;
}

uint16_t FLAKDECOMP::getShortFromLittleEndianBuffer(int start_index) {
    return (uint16_t) (this->buffer[start_index + 1] << 8) | this->buffer[start_index];
}

uint32_t FLAKDECOMP::getLongFromLittleEndianBuffer(int start_index) {
    return (uint32_t) (this->buffer[start_index + 3] << 24) | (this->buffer[start_index + 2] << 16) | 
        (this->buffer[start_index + 1] << 8) | this->buffer[start_index];
}

int16_t FLAKDECOMP::getSignedShortFromLittleEndianBuffer(int start_index) {
    return (int16_t) (this->buffer[start_index + 1] << 8) | this->buffer[start_index];
}