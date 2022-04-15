#include "FLAC.h"

using namespace std;

struct DataFrame<uint16_t> frame;

void FLACCOMP::compressFile(string file_name) {
    this->initialiseCompression(file_name, file_name);
    this->input_file.close();
    this->output_file.close();
}

void FLACCOMP::compressFile(string file_name, string destination_file) {
    this->initialiseCompression(file_name, destination_file);
    this->input_file.close();
    this->output_file.close();
}

// Detect file type, and initialise compression accordingly
void FLACCOMP::initialiseCompression(string file_name, string destination_file) {
    this->input_file.open(file_name, ios::binary);
    this->output_file.open(destination_file + ".flacc", ios::binary);
    this->pushToBuffer(4);
    string test = (char*) this->buffer.data();
    if (test == "RIFF") {
        this->pushToBuffer(8);
        string wave = (char*) &this->buffer.data()[8];
        if (wave == "WAVE") {
            this->compressWaveFile();
        } else {
            this->compressOtherFile();
        }
    } else {
        this->compressOtherFile();
    }
}

void FLACCOMP::compressWaveFile() {
    this->fillOutHeader();
    this->pushToBuffer(4);
    string data = (char*) &this->buffer.data()[this->buffer_end-4];
    if (data == "data") {
        while(fillOutFrame() == 0) {
            printf("Filled a whole frame!!\n");
            processFrame();
            writeFrame();
            break;
        }
    } else {
        printf("The data section is further away than normally!\n");
    }
}

void FLACCOMP::compressOtherFile() {
    printf("This is not a wave file!\n");
}

/*  Load frame_sample_size samples in to the buffer,
    and fill out the frame struct */
int FLACCOMP::fillOutFrame() {
    int ret = 0;
    for (int i = 0; i < this->frame_sample_size; i++) {
    // for (int i = 0; i < 2; i++) {
        ret = this->pushToBuffer(this->sample_byte_depth);
        if (ret != 0) break;
        // This line makes some assumptions, that breaks if the bit depth is not 16
        frame.left[i] = this->getSignedShortFromLittleEndianBuffer(this->buffer_end - this->sample_byte_depth);

        ret = this->pushToBuffer(this->sample_byte_depth);
        if (ret != 0) break;
        // This line makes some assumptions, that breaks if the bit depth is not 16
        frame.right[i] = this->getSignedShortFromLittleEndianBuffer(this->buffer_end - this->sample_byte_depth);
    }

    return ret;
}

void FLACCOMP::processFrame() {
    this->initialiseErrorArrays("left");
    printf("Processing!\n");
    this->initialiseErrorArrays("right");
}

void FLACCOMP::initialiseErrorArrays(string channel) {
    cout << "Initialising on channel " + channel + "\n";
    if (channel == "left") {
        for(int i = 0; i < 3; i++) {
            frame.e0[i] = frame.e1[i] = frame.e2[i] = frame.e3[i] = frame.left[i];
        }
    }
    else if (channel == "right") {
        for(int i = 0; i < 3; i++) {
            frame.e0[i] = frame.e1[i] = frame.e2[i] = frame.e3[i] = frame.right[i];
        }
    }
    for(int i = 1; i < 3; i++) {
        frame.e1[i] = frame.e0[i] - frame.e0[i-1];
        if (i > 1) {
            frame.e2[i] = frame.e1[i] - frame.e1[i-1];
        } if (i > 2) {
            frame.e3[i] = frame.e2[i] - frame.e2[i-1];
        }
    }
    // STILL NEED TO SUM UP THE FIRST 3 ERRORS
}

int FLACCOMP::writeFrame() {
    for (int i = 0; i < 2; i++) {
        printf("Left channel sample: %u\n", frame.left[i]);
        printf("Right channel sample: %u\n", frame.right[i]);
    }
    
    printf("Writing!\n");

    return 0;
}

int FLACCOMP::fillOutHeader() {
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
    // printf("Audioformat:        %u\n", this->wave_header.AudioFormat);
    // printf("Number of channels: %u\n", this->wave_header.NumChannels);
    // printf("Sample rate:        %u\n", this->wave_header.SampleRate);
    // printf("Byterate:           %u\n", this->wave_header.ByteRate);
    // printf("Block align:        %u\n", this->wave_header.BlockAlign);
    // printf("Bits per sample:    %u\n", this->wave_header.BitsPerSample);
    return (int) this->input_file.eof(); 
}

uint16_t FLACCOMP::getShortFromLittleEndianBuffer(int start_index) {
    return (uint16_t) (this->buffer[start_index + 1] << 8) | this->buffer[start_index];
}

uint32_t FLACCOMP::getLongFromLittleEndianBuffer(int start_index) {
    return (uint32_t) (this->buffer[start_index + 3] << 24) | (this->buffer[start_index + 2] << 16) | 
        (this->buffer[start_index + 1] << 8) | this->buffer[start_index];
}

int16_t FLACCOMP::getSignedShortFromLittleEndianBuffer(int start_index) {
    return (int16_t) (this->buffer[start_index + 1] << 8) | this->buffer[start_index];
}

int FLACCOMP::pushToBuffer(int n) {
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

void FLACCOMP::cleanBuffer() {
    // Hopefully at some point this will be implemented
}

void FLACCOMP::setFrameSampleSize(int size) {
    this->frame_sample_size = size;
}