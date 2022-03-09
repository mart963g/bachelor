#ifndef LZ77_H
#define LZ77_H
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class LZ77 {
    private:
        vector<unsigned char> buffer;
        int start_ind = 0;
        int end_ind = 10;
        int current_ind = 0;
    
    public:
        void testFunction();
};

#endif