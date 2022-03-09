#ifndef LZ77_H
#define LZ77_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

class LZ77 {
    private:
        vector<char> history_buffer;
        int history_buffer_size = 10;
        int history_buffer_index = 0;
        vector<char> lookahead_buffer;
        int lookahead_buffer_size = 5;
        int lookahead_buffer_index = 0;
        ifstream input_file;
        ofstream output_file;
        void moveBuffers();
        void cleanBuffers();
    
    public:
        void openFile(string file_name);
};

#endif