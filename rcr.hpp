#ifndef RCR_HPP
#define RCR_HPP

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

using namespace std;

typedef long long ll;
typedef unsigned long long ull;
enum CodeType : ull{NEWLINE, CONSTANT, RANGE};
struct Code{
    CodeType ct;
    ull delta;
    ull encode;
    int encode_length;
    Code(){}
    Code(CodeType ct, ull delta){
        this->ct = ct;
        this->delta = delta;
    }

    bool operator<(const Code& rhs) const{
        return tie(this->ct, this->delta) < tie(rhs.ct, rhs.delta);
    }

    bool operator==(const Code& rhs) const{
        return (this->ct == rhs.ct) && (this->delta == rhs.delta);
    }
};

struct Node{
    int left = -1;
    int right = -1;
    int frequency;
    Code code;
};

struct Options{
    Options(int argc, char* argv[]){
        cerr << "hello world" << endl;
        if(argc == 1)
            return;
        int currentArg = 1;
        int fileNamesRead = 0;
        while(currentArg < argc){
            string arg(argv[currentArg]);
            cerr << "processing arg: " << arg << endl;
            if(arg.substr(0,2) == "--"){
                if(arg.find("subheight") != string::npos)
                    this->subHeight = atoi(arg.substr(arg.find("=")+1,arg.size()).c_str());
                else if(arg.find("subwidth") != string::npos)
                    this->subWidth = atoi(arg.substr(arg.find("=")+1,arg.size()).c_str());
                else if(arg.find("huffmandepth") != string::npos)
                    this->huffmanEncodedDeltas = atoi(arg.substr(arg.find("=")+1,arg.size()).c_str());
                else if(arg.find("maxhuffmanlength") != string::npos)
                    this->maxHuffmanLength = atoi(arg.substr(arg.find("=")+1,arg.size()).c_str());
                cerr << "woot woot:" << this->subHeight << endl;

            }
            else if(arg[0] == '-'){
                cerr << "found -" << endl;
                //TODO: figure out find
                cerr << "location: " << arg.find("c") << endl;
                if(arg.find("c") != string::npos)
                    this->compress = true;
                if(arg.find("x") != string::npos)
                    this->compress = false;
            }else{
                if(fileNamesRead == 0){
                    this->inputFilename = arg;
                }else if(fileNamesRead == 1){
                    this->outputFilename = arg;
                }else{
                    cerr << "error" << endl;
                }
                fileNamesRead++;
            }
            currentArg++;
        }
        //TODO: print check
        cerr << "compress: " << this->compress << endl;
        cerr << "subHeight: " << this->subHeight << endl;
        cerr << "subWidth: " << this->subWidth << endl;
        cerr << "HuffmanCount: " << this->huffmanEncodedDeltas << endl;
        cerr << "inputFilename: " << this->inputFilename << endl;
        cerr << "outputFilename: " << this->outputFilename << endl;
    }
    bool compress=true;
    int subHeight=64;
    int subWidth=4;
    int huffmanEncodedDeltas=4;
    int maxHuffmanLength=-1;
    string inputFilename="";
    string outputFilename="";
    int N;
    int M;
    int nnz;
};

#endif
