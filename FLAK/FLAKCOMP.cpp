#include "FLAK.h"

using namespace std;

struct DataFrame<int16_t> frame;
struct ErrorWrapper<int16_t> errors;

void FLAKCOMP::compressFile(string file_name) {
    this->initialiseCompression(file_name, file_name);
    this->input_file.close();
    this->output_file.close();
}

void FLAKCOMP::compressFile(string file_name, string destination_file) {
    this->initialiseCompression(file_name, destination_file);
    this->input_file.close();
    this->output_file.close();
}

// Detect file type, and initialise compression accordingly
void FLAKCOMP::initialiseCompression(string file_name, string destination_file) {
    this->input_file.open(file_name, ios::binary);
    this->output_file.open(destination_file + ".flak", ios::binary);
    this->pushToBuffer(4);
    char* test = (char*) this->buffer.data();
    if (!memcmp(test, "RIFF", 4)) {
        this->pushToBuffer(8);
        char* wave = (char*) &this->buffer.data()[8];
        if (!memcmp(wave, "WAVE", 4)) {
            this->compressWaveFile();
        } else {
            this->compressOtherFile();
        }
    } else {
        this->compressOtherFile();
    }
}

void FLAKCOMP::compressWaveFile() {
    this->fillOutHeader();
    this->output_file.write("FLAK", 4);
    this->writeWaveHeader();
    int computed_samples = 0;
    int count = 0;
    while((computed_samples = this->fillOutFrame()) == 0) {
        // printf("Filled a whole frame!!\n");
        processFrame();
        count++;
        // break;
    }
    printf("Computed frames: %d\n", count);
    printf("Computed Samples: %d\n", computed_samples);
    if (computed_samples > 0) {
        this->processLastFrame(computed_samples);
    }
}

void FLAKCOMP::compressOtherFile() {
    printf("This is not a wave file!\n");
}

void FLAKCOMP::processLastFrame(int samples) {
    this->processLastSubFrame("left", samples);

    if (wave_header.NumChannels > 1) {
        this->processLastSubFrame("right", samples);
    }
}

void FLAKCOMP::processLastSubFrame(string channel, int samples) {
    this->initialiseErrorArrays(channel);
    this->processErrors(channel);
    errors.sums[0] = errors.sums[1] = errors.sums[2] = errors.sums[3] = 0;
    // Redo error sums, only including the actual samples from last frame
    for (int i = 0; i < samples; i++) {
        errors.sums[0] += abs(errors.e0[i]);
        errors.sums[1] += abs(errors.e1[i]);
        errors.sums[2] += abs(errors.e2[i]);
        errors.sums[3] += abs(errors.e3[i]);
    }
    long lowest = errors.sums[0];
    int index = 0;
    for (int i = 1; i < 4; i++) {
        if (errors.sums[i] < lowest) {
            index = i;
            lowest = errors.sums[i];
        }
    }
    this->writeSubFrameRaw(channel, index, samples);
}

/*  Load frame_sample_size samples in to the buffer,
    and fill out the frame struct */
int FLAKCOMP::fillOutFrame() {
    int ret;
    for (int i = 0; i < this->frame_sample_size; i++) {
    // for (int i = 0; i < 2; i++) {
        ret = this->pushToBuffer(this->sample_byte_depth);
        if (ret != 0) {
            return (i == 0 ? -1 : i);
        }
        // This function makes some assumptions, that breaks if the bit depth is not 16
        frame.left[i] = this->getSignedShortFromLittleEndianBuffer(this->buffer_end - this->sample_byte_depth);

        if (wave_header.NumChannels > 1) {
            ret = this->pushToBuffer(this->sample_byte_depth);
            if (ret != 0) {
                return (i == 0 ? -2 : i);
            }
            // This function makes some assumptions, that breaks if the bit depth is not 16
            frame.right[i] = this->getSignedShortFromLittleEndianBuffer(this->buffer_end - this->sample_byte_depth);
        }
    }

    return 0;
}

void FLAKCOMP::processFrame() {
    this->processSubFrame("left");
    if (wave_header.NumChannels > 1) {
        this->processSubFrame("right");
    }
}

void FLAKCOMP::processSubFrame(string channel) {
    this->initialiseErrorArrays(channel);
    this->processErrors(channel);
    // printf("E0 error sum: %ld\n", errors.sums[0]);
    // printf("E1 error sum: %ld\n", errors.sums[1]);
    // printf("E2 error sum: %ld\n", errors.sums[2]);
    // printf("E3 error sum: %ld\n", errors.sums[3]);
    long lowest = errors.sums[0];
    int index = 0;
    for (int i = 1; i < 4; i++) {
        if (errors.sums[i] < lowest) {
            index = i;
            lowest = errors.sums[i];
        }
    }
    // printf("Lowest error is order: %d with error: %ld\n", index, lowest);
    this->writeSubFrameRaw(channel, index);
    // Commented out for now
    // this->encodeResiduals(index);
}

/*  Fills out the first three entries in each of the
    error arrays, since these are special cases */
void FLAKCOMP::initialiseErrorArrays(string channel) {
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
void FLAKCOMP::processErrors(string channel) {
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
        errors.sums[0] += abs(errors.e0[i]);
        errors.sums[1] += abs(errors.e1[i]);
        errors.sums[2] += abs(errors.e2[i]);
        errors.sums[3] += abs(errors.e3[i]);
    }
    
}

void FLAKCOMP::writeSubFrameRaw(string channel, int order, int samples) {
    unsigned char write_channel = channel == "left" ? 0 : 1;
    // Sets the flag if samples is not the default number
    unsigned char last_subframe_flag = samples == this->frame_sample_size ? 0 : 1; 
    unsigned char write_order = order;
    unsigned char k = 0;
    unsigned char write = (last_subframe_flag << 7) | (write_channel << 6) | (write_order << 4) | k;
    // printf("Header char: %u\n", write);
    this->output_file.put(write);
    if (last_subframe_flag) {
        int16_t write_samples = samples;
        this->writeSignedShortToFile(write_samples);
    }
    for (int i = 0; i < order; i++) {
        if (channel == "left") {
            // printf("Error: %d\n", frame.left[i]);
            this->writeSignedShortToFile(frame.left[i]);
            // if (last_subframe_flag) {
            //     printf("COMP: Sample number %d in last subframe is: %d\n", i, frame.left[i]);
            //     printf("COMP: Real value is: %d\n", frame.left[i]);
            // }
        } else if (channel == "right"){
            // printf("Error: %d\n", frame.right[i]);
            this->writeSignedShortToFile(frame.right[i]);
            // if (last_subframe_flag) {
            //     printf("COMP: Sample number %d in last subframe is: %d\n", i, frame.right[i]);
            //     printf("COMP: Real value is: %d\n", frame.right[i]);
            // }
        }
    }
    // This doubled array is to avoid typing the whole class,
    // taking the generic template as a parameter in a function
    int16_t error_array[this->frame_sample_size];
    switch (order) {
        case 0:
            copy_n(errors.e0, this->frame_sample_size, error_array);
            break;
        case 1:
            copy_n(errors.e1, this->frame_sample_size, error_array);
            break;
        case 2:
            copy_n(errors.e2, this->frame_sample_size, error_array);
            break;
        case 3:
            copy_n(errors.e3, this->frame_sample_size, error_array);
            break;
        default:
            break;
    }
    for (int i = order; i < samples; i++) {
        // printf("Error: %d\n", error_array[i]);
        this->writeSignedShortToFile(error_array[i]);
        // if (i < 10 && last_subframe_flag) {
        //     printf("COMP: Sample number %d in last subframe is: %d\n", i, error_array[i]);
        //     printf("COMP: Real value is: %d\n", (channel == "left" ? frame.left[i] : frame.right[i]));
        //     i++;
        // }
    }
    
}

void FLAKCOMP::writeWaveHeader() {
    this->output_file.write((char*)this->buffer.data(), 44);
}

int FLAKCOMP::fillOutHeader() {
    this->wave_header.FileSize = this->getLongFromLittleEndianBuffer(4);
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
        printf("The data section is further away than normally!\n");
    }
    // printf("Audioformat:        %u\n", this->wave_header.AudioFormat);
    // printf("Number of channels: %u\n", this->wave_header.NumChannels);
    // printf("Sample rate:        %u\n", this->wave_header.SampleRate);
    // printf("Byterate:           %u\n", this->wave_header.ByteRate);
    // printf("Block align:        %u\n", this->wave_header.BlockAlign);
    // printf("Bits per sample:    %u\n", this->wave_header.BitsPerSample);
    return (int) this->input_file.eof(); 
}

uint16_t FLAKCOMP::getShortFromLittleEndianBuffer(int start_index) {
    return (uint16_t) (this->buffer[start_index + 1] << 8) | this->buffer[start_index];
}

uint32_t FLAKCOMP::getLongFromLittleEndianBuffer(int start_index) {
    return (uint32_t) (this->buffer[start_index + 3] << 24) | (this->buffer[start_index + 2] << 16) | 
        (this->buffer[start_index + 1] << 8) | this->buffer[start_index];
}

int16_t FLAKCOMP::getSignedShortFromLittleEndianBuffer(int start_index) {
    return (int16_t) (this->buffer[start_index + 1] << 8) | this->buffer[start_index];
}

int FLAKCOMP::pushToBuffer(int n) {
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

void FLAKCOMP::encodeResiduals(int order) {
    switch (order)
    {
    case 0:
        /* code */
        break;
    case 1:
        break;
    
    case 2:
        break;
    
    case 3:
        break;

    default:
        cout << "Error: Invalid predicter chosen!\n";
    }
}

void FLAKCOMP::writeSignedShortToFile(int16_t number) {
    unsigned char write = number & 255;
    // printf("First char: %u\n", write);
    this->output_file.put(write);
    write = (number >> 8) & 255;
    // printf("Second char: %u\n", write);
    this->output_file.put(write);
}

void FLAKCOMP::setFrameSampleSize(int size) {
    this->frame_sample_size = size;
}

void FLAKCOMP::cleanBuffer() {
    // Hopefully at some point this will be implemented
}