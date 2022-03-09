#ifndef LZ77_H
#define LZ77_H
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class LZ77 {
    private:
        vector<unsigned char> buffer;
        int start_ind;
        int end_ind;
        int current_ind;
    
    public:
        void testFunction();
};

#endif