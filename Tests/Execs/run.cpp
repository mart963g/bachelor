#include "LZ77.h"
#include <filesystem>

int main (int argc, char** argv) {
    printf("Compression phase...\n");
    for (int i = 1; i < argc; i++) {
        LZ77COMP comp;
        printf("Compressing %s \n", argv[i]);
        comp.compressFile("Tests/Files/" + string(argv[i]), "Tests/Compressed/" + string(argv[i]));
    }
    

    printf("\nDecompression phase...\n");
    for (int i = 1; i < argc; i++) {
        LZ77DECOMP decomp;
        printf("Decompressing %s \n", argv[i]);
        decomp.decompressFile("Tests/Compressed/" + string(argv[i]) + ".lzip", "Tests/Decompressed/" + string(argv[i]));
    }
    


    // std::ifstream test;
    // cout << "Printing numbers from file:\n";
    // test.open("Tests/Hello.txt.lzip", ios::binary);
    // test.seekg(0, ios::end);
    // int file_size = test.tellg();
    // test.seekg(0);

    // for(int i = 0; i < file_size; i++) {
    //     if (i % 10 == 0) cout << "\n";
    //     if (i % 2 == 0) {
    //         printf("O:%u\t", test.get());
    //     } else {
    //         printf("L:%u\t", test.get());
    //     }
    // }
    // cout << "\n";

    return 0;
}