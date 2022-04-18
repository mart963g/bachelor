#include "FLAC.h"

using namespace std;

struct DataFrame<int16_t> frame;
struct ErrorWrapper<int16_t> errors;

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
            // printf("Filled a whole frame!!\n");
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
    this->processSubFrame("left");
    this->processSubFrame("right");
}

void FLACCOMP::processSubFrame(string channel) {
    this->initialiseErrorArrays(channel);
    this->processErrors(channel);
    printf("E0 error sum: %ld\n", errors.e0_sum);
    printf("E1 error sum: %ld\n", errors.e1_sum);
    printf("E2 error sum: %ld\n", errors.e2_sum);
    printf("E3 error sum: %ld\n", errors.e3_sum);
    
}

/*  Fills out the first three entries in each of the
    error arrays, since these are special cases */
void FLACCOMP::initialiseErrorArrays(string channel) {
    // cout << "Initialising on channel " + channel + "\n";
    if (channel == "left") {
        for(int i = 0; i < 3; i++) {
            errors.e0[i] = errors.e1[i] = errors.e2[i] = errors.e3[i] = frame.left[i];
        }
    }
    else if (channel == "right") {
        for(int i = 0; i < 3; i++) {
            errors.e0[i] = errors.e1[i] = errors.e2[i] = errors.e3[i] = frame.right[i];
        }
    }
    for(int i = 1; i < 3; i++) {
        errors.e1[i] = errors.e0[i] - errors.e0[i-1];
        if (i > 1) {
            errors.e2[i] = errors.e1[i] - errors.e1[i-1];
        } if (i > 2) {
            errors.e3[i] = errors.e2[i] - errors.e2[i-1];
        }

    }
}

// Fills out the error arrays, and calculates the error sums
void FLACCOMP::processErrors(string channel) {
    if (channel == "left") {
        for (int i = 3; i < this->frame_sample_size; i++){
            errors.e0[i] = frame.left[i];
            errors.e1[i] = errors.e0[i] - errors.e0[i-1];
            errors.e2[i] = errors.e1[i] - errors.e1[i-1];
            errors.e3[i] = errors.e2[i] - errors.e2[i-1];
        }
    }
    else if (channel == "right") {
        for (int i = 3; i < this->frame_sample_size; i++){
            errors.e0[i] = frame.right[i];
            errors.e1[i] = errors.e0[i] - errors.e0[i-1];
            errors.e2[i] = errors.e1[i] - errors.e1[i-1];
            errors.e3[i] = errors.e2[i] - errors.e2[i-1];
        }
    }
    // Codewise it is prettier to separate this as here, but it is less efficient
    // since it requires more memory reads, that are probably not cached
    for (int i = 0; i < this->frame_sample_size; i++) {
        errors.e0_sum += abs(errors.e0[i]);
        errors.e1_sum += abs(errors.e1[i]);
        errors.e2_sum += abs(errors.e2[i]);
        errors.e3_sum += abs(errors.e3[i]);
    }
    
}

int FLACCOMP::writeFrame() {
    // for (int i = 0; i < 10; i++) {
    //     cout << "Left channel sample: " << frame.left[i] << "\n";
    //     cout << "Right channel sample: " << frame.right[i] << "\n";
    // }
    
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