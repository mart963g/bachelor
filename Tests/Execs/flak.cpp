#include "FLAK.h"

int main (int argc, char** argv) {
    printf("Compression phase FLAK...\n");
    for (int i = 1; i < argc; i++) {
        FLAKCOMP comp;
        printf("Compressing %s \n", argv[i]);
        comp.compressFile("Tests/Files/" + string(argv[i]), "Tests/Compressed/" + string(argv[i]));
    }    

    // printf("\nDecompression phase FLAK...\n");
    // for (int i = 1; i < argc; i++) {
    //     FLAKDECOMP decomp;
    //     printf("Decompressing %s \n", argv[i]);
    //     decomp.decompressFile("Tests/Compressed/" + string(argv[i]) + ".flak", "Tests/Decompressed/" + string(argv[i]));
    // }

    return 0;
}