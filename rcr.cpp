#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <math.h>
#include "rcr.hpp"
#include "rcrHelper.hpp"

//TODO: newlines
//TODO: support very large deltas
//gamma code after greater than 64
//use 16 codes

int main(int argc, char* argv[]){
    Options mainOptions(argc,argv);
    if(mainOptions.compress)
        return compress(mainOptions);
    else
        return extract(mainOptions);
}
