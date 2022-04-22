#include "FLAK.h"

using namespace std;

struct DataFrame<int16_t> data_frame;

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
    printf("This was a wave file!\n");
    this->fillOutHeader();
    this->writeWaveHeader();
    int computed_samples = 0;
    while((computed_samples = this->readFrame()) == 0) {
        this->processFrame();
    }

}

void FLAKDECOMP::decompressOtherFile() {
    printf("This was not a wave file!\n");
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
        return -1;
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
    int ret;
    for (int i = 0; i < this->frame_sample_size; i++) {
    // for (int i = 0; i < 2; i++) {
        ret = this->pushToBuffer(this->sample_byte_depth);
        if (ret != 0) {
            return (i == 0 ? -1 : i);
        }
        // This function makes some assumptions, that breaks if the bit depth is not 16
        data_frame.left[i] = this->getSignedShortFromLittleEndianBuffer(this->buffer_end - this->sample_byte_depth);

        if (wave_header.NumChannels > 1) {
            ret = this->pushToBuffer(this->sample_byte_depth);
            if (ret != 0) {
                return (i == 0 ? -2 : i);
            }
            // This function makes some assumptions, that breaks if the bit depth is not 16
            data_frame.right[i] = this->getSignedShortFromLittleEndianBuffer(this->buffer_end - this->sample_byte_depth);
        }
    }

    return 0;
}

void FLAKDECOMP::processFrame() {
    this->pushToBuffer(1);
    unsigned char header_char = this->buffer[this->buffer_end - 1]; 
    unsigned char write_channel = (header_char & 192) >> 6;
    unsigned char write_order = (header_char >> 4) & 3;
    unsigned char k = header_char & 15;
    string channel = write_channel == 0 ? "left" : "right";
    printf("Header char: %u\t\t", header_char);
    cout << "Channel: " + channel;
    printf("\tOrder: %u\tK: %u\n", channel, write_order, k);
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