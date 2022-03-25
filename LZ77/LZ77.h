#ifndef LZ77_H
#define LZ77_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

struct token {
    uint16_t offset;
    unsigned char length;
};


struct nonMatchToken {
    unsigned char non_match : 1;
    unsigned char offset : 7;
    unsigned char length;
}; 

/*  The matchToken is the one that there could potentially be several
    similar types of, with different bit settings */
struct matchToken {
    unsigned char non_match : 1;
    uint16_t offset : 10;
    unsigned char length : 5;
};

class LZ77COMP {
    private:
        int buffer_max_size = 2048;
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
        int initialiseCompression(string file_name);
        int initialiseCompression(string file_name, string destination_file);
        // The history buffer can max be of size 1024-1, and lookahead max is 32-1
        int initialiseBuffers(int history_size = 1023, int lookahead_size = 31);
        void compressAfterInitialisation();
        struct token searchForMatch();
        struct matchToken getMatchToken(struct token token);
        struct nonMatchToken getNonMatchToken(struct token token);
        void finishCompression();
    
    public:
        void compressFile(string file_name);
        void compressFile(string file_name, string destination_file);
        void setBufferMaxSize(int size);
};

class LZ77DECOMP {
    private:
        int buffer_max_size = 1024;
        // Contains all the previously processed bytes
        vector<unsigned char> buffer;
        // End is excluded. If it is > 0, the last element is at index (end - 1)
        int buffer_end = 0;
        ifstream input_file;
        ofstream output_file;
        void initialiseDecompression(string file_name);
        void initialiseDecompression(string file_name, string destination_file);
        void decompressAfterInitialisation();
        // Returns non zero on EOF, sets token values on succesfull read
        int readTokenFromInput(struct token &token);
        void writeBytesFromToken(struct token token);
        void moveBuffer(int steps = 1);
        void cleanBuffer();
        struct token getTokenFromMatchToken(struct matchToken token);
        struct token getTokenFromNonMatchToken(struct nonMatchToken token);

    public:
        void decompressFile(string file_name);
        void decompressFile(string file_name, string destination_file);
        void setBufferMaxSize(int size);
};

#endif