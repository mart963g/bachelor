#ifndef LZ77_H
#define LZ77_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

struct token {
    unsigned char offset;
    unsigned char length;
};

class LZ77 {
    private:
        int buffer_max_size = 1024;
        // Contains all the bytes, where possible matches are found for the lookahead buffer
        vector<unsigned char> history_buffer;
        // End is excluded. If it is > 0, the last element is at index (end - 1)
        int history_buffer_end = 0;
        int history_buffer_start = 0;
        // Contains all the bytes for which a possible match is searched for
        vector<unsigned char> lookahead_buffer;
        // End is excluded. If it is > 0, the last element is at index (end - 1)
        int lookahead_buffer_end = 0;
        int lookahead_buffer_start = 0;
        ifstream input_file;
        ofstream output_file;
        int moveBuffers(int steps = 1);
        void cleanBuffers();
        void incrementIndexes();
        void writeTokenToOutputFile(struct token token);
        void initialiseCompression(string file_name);
        void initialiseBuffers(int history_size = 10, int lookahead_size = 5);
        void compressAfterInitialisation();
        struct token searchForMatch();
    
    public:
        void compressFile(string file_name);
        void setBufferMaxSize(int size);
};

#endif