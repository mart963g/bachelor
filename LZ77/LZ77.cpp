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
        void testFunction() {
            cout << "Hello World!\n" << end_ind << "\n";
        }



};



int main () {
    LZ77 comp;
    comp.testFunction();

    return 0;
}