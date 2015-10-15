#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "spm.hpp"

using namespace std;

int main(int argc, char* argv[]){
    SpmOptions options(argc, argv);
    cerr << "compress: " << options.compress << endl;
    if(options.compress)
        return spmCompress(options);
    else
        return spmDecompress(options);
}
