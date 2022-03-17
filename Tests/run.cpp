#include "LZ77.h"
#include <filesystem>

int main () {
    LZ77 comp;
    comp.compressFile("Tests/Hello.txt");

    std::ifstream test;
    cout << "Printing numbers from file:\n";
    test.open("Tests/Hello.txt.lzip", ios::binary);
    test.seekg(0, ios::end);
    int file_size = test.tellg();
    test.seekg(0);

    for(int i = 0; i < file_size; i++) {
        if (i % 10 == 0) cout << "\n";
        if (i % 2 == 0) {
            printf("O:%u\t", test.get());
        } else {
            printf("L:%u\t", test.get());
        }
    }
    cout << "\n";

    return 0;
}