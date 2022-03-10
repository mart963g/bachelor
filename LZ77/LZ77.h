#ifndef LZ77_H
#define LZ77_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

class LZ77 {
    private:
        int buffer_max_size = 1024;
        vector<char> history_buffer;
        int history_buffer_end;
        int history_buffer_start = 0;
        vector<char> lookahead_buffer;
        int lookahead_buffer_end;
        int lookahead_buffer_start = 0;
        ifstream input_file;
        ofstream output_file;
        void moveBuffers(int steps = 1);
        void cleanBuffers();
        void incrementIndexes();
    
    public:
        void openFile(string file_name);
        void initialiseBuffers(int history_size = 10, int lookahead_size = 5);
        void setBufferMaxSize(int size);
};

#endif