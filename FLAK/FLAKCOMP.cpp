#include "FLAK.h"

using namespace std;

struct DataFrame<int16_t> frame;
struct ErrorWrapper<int16_t> errors;
char compression_different_sample_number_in_channels_flag = 0;

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
    this->rice_16.setOutputFile(&this->output_file);
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
    // printf("Computed frames: %d\n", count);
    // printf("Computed Samples: %d\n", computed_samples);
    if (computed_samples > 0) {
        this->processLastFrame(computed_samples);
    }
}

void FLAKCOMP::compressOtherFile() {
    // printf("This is not a wave file!\n");
    this->output_file.write("FLAk", 4);
    int computed_samples = this->fillOutFirstFrameNonWave();
    // printf("Computed samples: %d\n", computed_samples);
    if (computed_samples == 0) {
        this->processFrame();
        // int count = 0;
        while((computed_samples = this->fillOutFrame()) == 0) {
            // printf("Filled a whole frame!!\n");
            this->processFrame();
            // count++;
            // break;
        }
    }
    // printf("Computed frames: %d\n", count);
    // printf("Computed Samples: %d\n", computed_samples);
    if (computed_samples > 0) {
        this->processLastFrame(computed_samples);
    }
}

void FLAKCOMP::processLastFrame(int samples) {
    // If the flag is set, there is an extra sample in the left subframe.
    if (compression_different_sample_number_in_channels_flag) {
        this->processLastSubFrame("left", samples + 1);
    } else {
        this->processLastSubFrame("left", samples);
    }

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

    // printf("Lowest error is order: %d with error: %ld\n", index, lowest);
    int m = static_cast<int>(log2(log(2) * (errors.sums[index]/samples)));
    if (m > this->wave_header.BitsPerSample) {
        m = this->wave_header.BitsPerSample;
    }
    // int k = static_cast<int> (ceil(log2(errors.sums[index]/(samples))));
    this->writeSubFrameHeader(channel, index, m, samples);
    // Commented out for now
    this->writeSubFrameResiduals(channel, index, m, samples);

    // this->writeSubFrameRaw(channel, index, samples);
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
        if (this->sample_byte_depth > 1) {
            frame.left[i] = this->getSignedShortFromLittleEndianBuffer(this->buffer_end - this->sample_byte_depth);
        } else {
            frame.left[i] = this->buffer[this->buffer_end-1];
        }

        if (wave_header.NumChannels > 1) {
            ret = this->pushToBuffer(this->sample_byte_depth);
            if (ret != 0) {
                // Sets the flag, to signal that there were more samples in the first channel than in the second
                compression_different_sample_number_in_channels_flag = 1;
                // printf("Different sample number in channels detected!\n");
                // printf("Last sample has value: %d\n", frame.left[i]);
                return (i == 0 ? -2 : i);
            }
            if (this->sample_byte_depth > 1) {
                frame.right[i] = this->getSignedShortFromLittleEndianBuffer(this->buffer_end - this->sample_byte_depth);
            } else {
                frame.right[i] = this->buffer[this->buffer_end-1];
            }
        }
    }

    return ret;
}

// Fills out the frame the first time, if the file being compressed is not a WAVE file.
// Assumes that the sample bit depth is 8.
int FLAKCOMP::fillOutFirstFrameNonWave() {
    int start_size = this->buffer.size();
    // Initialise frame with bytes already in the buffer
    for (int i = 0; i < start_size; i++) {
        frame.left[i] = this->buffer[i];
    }
    
    int ret = 0;
    // Fill out the rest of the trame
    for (int i = start_size; i < this->frame_sample_size; i++) {
        ret = this->pushToBuffer(this->sample_byte_depth);
        if (ret != 0) {
            return (i == 0 ? -1 : i);
        }
        frame.left[i] = this->buffer[this->buffer_end-1];
    }
    return ret;
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
    int m = static_cast<int>(log2(log(2) * (errors.sums[index]/this->frame_sample_size)));
    // This part was breaking the non wave files, since m could be 16 sometimes, 
    // which would break the reading/writing of residuals since they only had 8 bits.
    if (m > this->wave_header.BitsPerSample) {
        m = this->wave_header.BitsPerSample;
    }
    // The header can not store m values > 15.
    if (m >= 16) {
        m = 15;
    }
    // int k = static_cast<int> (ceil(log2(errors.sums[index]/(frame_sample_size_const))));
    this->writeSubFrameHeader(channel, index, m);
    // Commented out for now
    this->writeSubFrameResiduals(channel, index, m);
    // this->writeSubFrameRaw(channel, index);
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

void FLAKCOMP::writeSubFrameHeader(string channel, int order, int m, int samples) {
    unsigned char write_channel = channel == "left" ? 0 : 1;
    // Sets the flag if samples is not the default number
    unsigned char last_subframe_flag = samples == this->frame_sample_size ? 0 : 1; 
    unsigned char write_order = order;
    unsigned char write_m = m;
    unsigned char write = (last_subframe_flag << 7) | (write_channel << 6) | (write_order << 4) | write_m;
    this->output_file.put(write);
    if (last_subframe_flag) {
        int16_t write_samples = samples;
        this->writeSignedShortToFile(write_samples);
    }
    for (int i = 0; i < order; i++) {
        if (channel == "left") {
            if (this->sample_byte_depth > 1) {
                this->writeSignedShortToFile(frame.left[i]);
            } else {
                this->output_file.put((char) frame.left[i]);
            }
        } else if (channel == "right"){
            if (this->sample_byte_depth > 1) {
                this->writeSignedShortToFile(frame.right[i]);
            } else {
                this->output_file.put((char) frame.right[i]);
            }
        }
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
        // printf("Order: %d\n", order);
    }
    for (int i = 0; i < order; i++) {
        if (channel == "left") {
            // printf("Error: %d\n", frame.left[i]);
            if (this->sample_byte_depth > 1) {
                this->writeSignedShortToFile(frame.left[i]);
            } else {
                this->output_file.put((char) frame.left[i]);
            }
            // if (i < 20 && this->buffer.size() < 1000) {
            //     printf("COMP: Sample number %d in last subframe is: %d\n", i, frame.left[i]);
            //     printf("COMP: Real value is: %d\n", frame.left[i]);
            // }
        } else if (channel == "right"){
            // printf("Error: %d\n", frame.right[i]);
            if (this->sample_byte_depth > 1) {
                this->writeSignedShortToFile(frame.right[i]);
            } else {
                this->output_file.put((char) frame.right[i]);
            }
            // if (last_subframe_flag) {
                // printf("COMP: Sample number %d in last subframe is: %d\n", i, frame.right[i]);
                // printf("COMP: Real value is: %d\n", frame.right[i]);
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
        if (this->sample_byte_depth > 1) {
            this->writeSignedShortToFile(error_array[i]);
        } else {
            this->output_file.put((char) error_array[i]);
        }
        // if (i < 20 && this->buffer.size() < 1000) {
        //     printf("COMP: Sample number %d in last subframe is: %d\n", i, error_array[i]);
        //     printf("COMP: Real value is: %d\n", (channel == "left" ? frame.left[i] : frame.right[i]));
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

void FLAKCOMP::writeSubFrameResiduals(string channel, int order, int k, int samples) {
    int residual_samples = samples - order;
    int16_t error_array[residual_samples];
    switch (order) {
        case 0:
            copy_n(errors.e0 + order, residual_samples, error_array);
            break;
        case 1:
            copy_n(errors.e1 + order, residual_samples, error_array);
            break;
        case 2:
            copy_n(errors.e2 + order, residual_samples, error_array);
            break;
        case 3:
            copy_n(errors.e3 + order, residual_samples, error_array);
            break;
        default:
            break;
    }
    // printf("Order: %d\n", order);
    // for (int i = 0; i < 20; i++) {
    //     printf("Error number %d is: %d\n", i, errors.e1[i]);
    // }
    // exit(-1);
    // The log_2 of (log_e(2) multiplied with the expected value of (absolute value of) the error)
    // int m = log2(log(2) * (errors.sums[order]/residual_samples));
    // printf("M is: %d\n", m);
    this->rice_16.encodeSubFrame(error_array, residual_samples, k);
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