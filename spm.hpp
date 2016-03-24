#ifndef SPM_H
#define SPM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include "constants.hpp"

using namespace std;

typedef long long ll;
typedef unsigned long long ull;
//enum CodeType : ull{NEWLINE, CONSTANT, RANGE};
//TODO: change to 8 bytes
struct SpmCode{
    ull encode_length : NBITS_SPM_CODE_ENCODE_BITS;
    enum CodeType {
        NEWLINE, CONSTANT, RANGE
    }ct : 2;
    ull delta : 5;
    ull encode : SPM_CODE_ENCODE_BITS;
    SpmCode(){}
    SpmCode(CodeType ct, ull delta){
        this->ct = ct;
        this->delta = delta;
    }

    bool operator<(const SpmCode& rhs) const{
        if(this->ct == rhs.ct)
            return this->delta < rhs.delta;
        else
            return this->ct < rhs.ct;
        //return tie(this->ct, this->delta) < tie(rhs.ct, rhs.delta);
    }

    bool operator==(const SpmCode& rhs) const{
        return (this->ct == rhs.ct) && (this->delta == rhs.delta);
    }
};

struct SpmNode{
    int left;
    int right;
    int frequency;
    SpmCode code;
    SpmNode(){
        left = -1;
        right = -1;
    }
};

struct SpmOptions{
    SpmOptions(int argc, char* argv[]){
        compress=true;
        subHeight=256;
        subWidth=16;
        huffmanEncodedDeltas=4;
        maxHuffmanLength=-1;
        inputFilename="";
        outputFilename="";
        if(argc == 1)
            return;
        int currentArg = 1;
        int fileNamesRead = 0;
        while(currentArg < argc){
            string arg(argv[currentArg]);
            if(arg.substr(0,2) == "--"){
                if(arg.find("subheight") != string::npos)
                    this->subHeight = atoi(arg.substr(arg.find("=")+1,arg.size()).c_str());
                else if(arg.find("subwidth") != string::npos)
                    this->subWidth = atoi(arg.substr(arg.find("=")+1,arg.size()).c_str());
                else if(arg.find("huffmandepth") != string::npos)
                    this->huffmanEncodedDeltas = atoi(arg.substr(arg.find("=")+1,arg.size()).c_str());
                else if(arg.find("maxhuffmanlength") != string::npos)
                    this->maxHuffmanLength = atoi(arg.substr(arg.find("=")+1,arg.size()).c_str());

            }
            else if(arg[0] == '-'){
                if(arg.find("c") != string::npos)
                    this->compress = true;
                if(arg.find("x") != string::npos)
                    this->compress = false;
                if(arg.find("d") != string::npos)
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
    }
    bool compress;
    int subHeight;
    int subWidth;
    int huffmanEncodedDeltas;
    int maxHuffmanLength;
    string inputFilename;
    string outputFilename;
    int N;
    int M;
    int nnz;
};
typedef long long ll;
vector<SpmCode> createCodes(map<SpmCode, ll> &distribution, ll nnz = -1, ll maxLength = 32);
ull BitsToInt(vector<bool> bits);
vector<ll> decode(vector<ull> stream, vector<ull> argumentStream, vector<SpmCode> codes, ll length, ll argumentLength);
bool writeToFile(SpmOptions &mainOptions, vector<ull> &stream, vector<ull> &argumentStream, vector<SpmCode> &codes, ll length, ll argumentLength, string filename);
bool writeToFile(SpmOptions &mainOptions, vector<ull> &stream, vector<ull> &argumentStream, vector<SpmCode> &codes, ll length, ll argumentLength, FILE* output);
bool readFromFile(SpmOptions &mainOptions, vector<ull> &stream, vector<ull> &argumentStream, vector<SpmCode> &codes, ll &length, ll &argumentLength, string filename);
bool checkEquality(vector<ull> &streamOld, vector<ull> &streamNew, vector<SpmCode> &codesOld, vector<SpmCode> &codesNew);

int compress(SpmOptions mainOptions);
int extract(SpmOptions mainOptions);
int FindMaxCodeLength(vector<SpmCode> codes);

int FindMaxCodeLength(vector<SpmCode> codes){
    int max = 0;
    for(auto it = codes.begin(); it != codes.end(); ++it){
        if(max < it->encode_length)
            max = it->encode_length;
    }
    return max;
}

bool compare(SpmNode left, SpmNode right){
    return left.frequency < right.frequency;
}

char buffer[200];

ull BitsToInt(vector<bool> bits){
    ull ret = 0;
    if(bits.size() > 64)
        cerr << "ERROR: too many bits" << endl;
    for(ull i = 0; i < bits.size(); ++i){
        if(bits[i])
            ret |= 1LL << i;
    }
    return ret;
}
vector<SpmCode> createCodes(map<SpmCode, ll> &distribution, ll nnz, ll maxLength){
    vector<SpmCode> codes;
    if(distribution.size() == 0)
        return codes;
    vector<SpmNode> tree;
    for(auto it = distribution.begin(); it != distribution.end(); ++it){
        SpmNode tmp;
        tmp.frequency = it->second;
        if(nnz != -1){
            if(tmp.frequency < nnz >> maxLength)
                tmp.frequency = nnz >> maxLength;
        }
        tmp.code = it->first;
        tree.push_back(tmp);
    }
    if(tree.size() == 0)
        tree.push_back(SpmNode());
    for(int i = 0; i < tree.size()-1; i += 2){
        sort(tree.begin()+i,tree.end(),compare);
        SpmNode tmp;
        tmp.left = i;
        tmp.right = i+1;
        tmp.frequency = tree[i].frequency + tree[i+1].frequency;
        tree.push_back(tmp);
    }

    //create codes
    vector<bool> path;
    vector<int> parentStack;
    vector<int> phase;
    int curr = tree.size() - 1;
    phase.push_back(0);
    while(phase[0] != 2 || phase.size() != 1){
        switch(phase.back()){
            case 0:
                phase[phase.size()-1] = 1;
                if(tree[curr].left != -1){
                    path.push_back(0);
                    parentStack.push_back(curr);
                    phase.push_back(0);
                    curr = tree[curr].left;
                }
                break;
            case 1:
                phase[phase.size()-1] = 2;
                if(tree[curr].right != -1){
                    path.push_back(1);
                    parentStack.push_back(curr);
                    phase.push_back(0);
                    curr = tree[curr].right;
                }
                break;
            case 2:
                if(tree[curr].left == -1 && tree[curr].right == -1){
                    if(path.size() > SPM_CODE_ENCODE_BITS){
                        cerr << "code too long" << endl;
                        //exit(1);
                    }
                    SpmCode tmp = tree[curr].code;
                    tmp.encode = BitsToInt(path);
                    tmp.encode_length = path.size();
                    codes.push_back(tmp);
                }
                phase.pop_back();
                curr=parentStack.back();
                parentStack.pop_back();
                path.pop_back();
                break;
            default:
                break;
        }
    }
    return codes;
}
    //TODO: print rcr file
    //number of codes
bool writeToFile(SpmOptions &mainOptions, vector<ull> &stream, vector<ull> &argumentStream, vector<SpmCode> &codes, ll length, ll argumentLength, FILE* output){
    ull tmp = codes.size();
    char* printerPtr = (char*)&tmp;
    tmp = mainOptions.M;
    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i)
        fprintf(output, "%c", printerPtr[i]);
    tmp = mainOptions.N;
    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i)
        fprintf(output, "%c", printerPtr[i]);
    tmp = mainOptions.nnz;
    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i)
        fprintf(output, "%c", printerPtr[i]);
    //TODO: bits for Huffman
    //TODO: bits for arguments
    //TODO: codes start address
    //TODO: huffman start address
    //TODO: arguments start address
    //TODO: end address

    tmp = codes.size();
    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i){
        fprintf(output, "%c", printerPtr[i]);
    }
    for(int i = 0; i < codes.size(); ++i){
        //TODO: print code type
        tmp = codes[i].ct;
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j)
            fprintf(output, "%c", printerPtr[j]);
        tmp = codes[i].encode;
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j)
            fprintf(output, "%c", printerPtr[j]);
        tmp = codes[i].encode_length;
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j)
            fprintf(output, "%c", printerPtr[j]);
        tmp = codes[i].delta;
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j)
            fprintf(output, "%c", printerPtr[j]);
    }
    //codes
    //bits in stream
    tmp = length;
    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i){
        fprintf(output, "%c", printerPtr[i]);
    }
    for(int i = 0; i < stream.size(); ++i){
        tmp = stream[i];
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j){
            fprintf(output, "%c", printerPtr[j]);
        }
    }
    //stream
    //TODO: argument stream
    tmp = argumentLength;
    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i){
        fprintf(output, "%c", printerPtr[i]);
    }
    for(int i = 0; i < argumentStream.size(); ++i){
        tmp = argumentStream[i];
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j){
            fprintf(output, "%c", printerPtr[j]);
        }
    }
    fclose(output);
    return true;
}
bool writeToFile(SpmOptions &mainOptions, vector<ull> &stream, vector<ull> &argumentStream, vector<SpmCode> &codes, ll length, ll argumentLength, string filename){
    FILE* output = fopen(filename.c_str(),"w");
    return writeToFile(mainOptions, stream, argumentStream, codes, length, argumentLength, output);
}


int spmDecompress(vector<ull> &row, vector<ull> &col, vector<SpmCode> &spmCodes, vector<ull> &encodedStream, vector<ull> &argumentStream, ull length, ull argumentLength, int subHeight = SUB_HEIGHT, int subWidth = SUB_WIDTH);

int spmDecompress(SpmOptions options){
    FILE* outputFile;
    if(options.outputFilename == ""){
        outputFile = stdout;
    }else{
        outputFile = fopen(options.outputFilename.c_str(), "w");
    }
    vector<ull> encodedStream;
    vector<ull> encodedArgumentStream;
    vector<SpmCode> codes;
    ll length;
    ll argumentLength;
    readFromFile(options, encodedStream, encodedArgumentStream, codes, length, argumentLength, options.inputFilename);

    vector<ull> row;
    vector<ull> col;
    spmDecompress(row, col, codes, encodedStream, encodedArgumentStream, length, argumentLength);

    fprintf(outputFile, "\%\%MatrixMarket matrix coordinate pattern general\n");
    fprintf(outputFile, "%lld %lld %lld\n", options.M, options.N, options.nnz);
    for(ull i = 0; i < options.nnz; ++i)
        fprintf(outputFile, "%lld %lld\n", row[i] + 1, col[i] + 1);
    return 0;
}
int spmDecompress(vector<ull> &row, vector<ull> &col, vector<SpmCode> &spmCodes, vector<ull> &encodedStream, vector<ull> &encodedArgumentStream, ull length, ull argumentLength, int subHeight, int subWidth){
    vector<ll> decodedDeltas = decode(encodedStream, encodedArgumentStream, spmCodes, length, argumentLength);
    ll x = -1;
    ll y  = 0;
    ll newLines = 0;
    map<ll, map<ll,bool> > mapIndices;
    for(int i = 0; i < decodedDeltas.size(); ++i){
        if(decodedDeltas[i] == -1){
            newLines++;
            x = -1;
            y = newLines * subHeight;
        }else{
            ll leastSignificant = (decodedDeltas[i] + 1 + x % subWidth) % subWidth;
            ll mostSignificant = (decodedDeltas[i] + 1 + x % subWidth) / subWidth;
            x = (x / subWidth) * subWidth + leastSignificant;
            leastSignificant = (mostSignificant + y % subHeight) % subHeight;
            mostSignificant = (mostSignificant + y % subHeight) / subHeight;
            //TODO: finish
            y = (y / subHeight) * subHeight + leastSignificant;
            x += mostSignificant * subWidth;
            row.push_back(y);
            col.push_back(x);
            mapIndices[y][x] = true;
        }
    }
    return 0;
    for(auto it1 = mapIndices.begin(); it1 != mapIndices.end(); ++it1)
        for(auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2){
            row.push_back(it1->first);
            col.push_back(it2->first);
        }

}

int spmCompress(vector<ull> &row, vector<ull> &col, vector<SpmCode> &spmCodes, vector<ull> &encodedStream, vector<ull> &argumentStream, ull &length, ull &argumentLength, int subRow = SUB_HEIGHT, int subCol = SUB_WIDTH, int huffmanCodesSize = CONSTANT_DELTAS, int maxHuffmanLength = SPM_CODE_ENCODE_BITS);

int spmCompress(vector<ull> &row, vector<ull> &col, vector<SpmCode> &spmCodes, vector<ull> &encodedStream, vector<ull> &argumentStream, ull &length, ull &argumentLength, int subRow, int subCol, int huffmanCodesSize, int maxHuffmanLength){
    spmCodes.clear();
    encodedStream.clear();
    argumentStream.clear();
    int nnz = row.size();
    map<ll, map<ll, map<ll, map<ll, pair<int,int> > > > > matrix;
    for(ll i = 0; i < nnz; ++i){
        matrix[row[i]/subRow][col[i]/subCol][row[i]%subRow][col[i]%subCol] = make_pair(row[i], col[i]);
    }
    vector<ll> deltas;
    ll delta = 0;
    ll p1 = 0; ll p2 = 0; ll p3 = 0; ll p4 = 0;
    int i = 0;
    i = 0;
    for(auto i1 = matrix.begin(); i1 != matrix.end(); ++i1){
        i++;
        for(int i = 0; i < i1->first - p1; ++i){
            deltas.push_back(-1);
        }
        for(auto i2 = i1->second.begin(); i2 != i1->second.end(); ++i2){
            delta += (i2->first - p2)*subRow*subCol;
            for(auto i3 = i2->second.begin(); i3 != i2->second.end(); ++i3){
                delta += (i3->first - p3)*subCol;
                for(auto i4 = i3->second.begin(); i4 != i3->second.end(); ++i4){
                    delta += i4->first - p4;
                    if(delta >= 1LL << 31){
                        cerr << "overflowing delta: " << delta << endl;
                        exit(2);
                    }
                    deltas.push_back(delta);
                    delta = -1;
                    p4 = i4->first;
                }
                p3 = i3->first;
            }
            p2 = i2->first;
        }
        p1 = i1->first;
        delta = 0;
        p2=0; p3=0; p4=0;
    }
    map<SpmCode, ll> distribution;
    for(int i = 0; i < deltas.size(); ++i){
        if(deltas[i] == -1)
            distribution[SpmCode(SpmCode::NEWLINE, 0)]++;
        else if(deltas[i] < huffmanCodesSize)
            distribution[SpmCode(SpmCode::CONSTANT,deltas[i])]++;
        else
            distribution[SpmCode(SpmCode::RANGE,(ll)log2(deltas[i]))]++;
    }
    vector<SpmCode> codes;
    //maxHuffmanLength = 6;
    if(maxHuffmanLength != -1){
        int codeLengthTarget = maxHuffmanLength;
        for(;;){
            codes = createCodes(distribution, nnz, codeLengthTarget);
            if(FindMaxCodeLength(codes) > maxHuffmanLength)
                codeLengthTarget--;
            else
                break;
        }
    }else{
        //TODO: unbounded case
    }
        //codes = createCodes(distribution);
    int maxCodeLength = FindMaxCodeLength(codes);
    if(maxCodeLength > maxHuffmanLength || maxCodeLength == 0) {
        cerr << "code length greater than max" << endl;
        cerr << "maxCodeLength: " << maxCodeLength << endl;
        exit(1);
    }
    map<SpmCode, SpmCode> codeMap;
    for(int i = 0; i < codes.size(); ++i){
        codeMap[codes[i]] = codes[i];
    }
    ll encodedCurrBit = 0;
    ll encodedLatest = 0;
    ll argumentCurrBit = 0;
    ll argumentLatest = 0;
    for(ll i = 0; i < deltas.size(); ++i){
        ll delta = deltas[i];
        SpmCode deltaCode;
        ll deltaArgument = 0;
        if(delta == -1)
            deltaCode = SpmCode(SpmCode::NEWLINE, 0);
        else if(delta >= huffmanCodesSize)
            deltaCode = SpmCode(SpmCode::RANGE, (ll)log2(delta));
        else
            deltaCode = SpmCode(SpmCode::CONSTANT, delta);
        encodedLatest |= (ull)codeMap[deltaCode].encode << encodedCurrBit;
        if(encodedCurrBit + codeMap[deltaCode].encode_length == 64){
            encodedStream.push_back(encodedLatest);
            encodedLatest = 0;
            encodedCurrBit = 0;
        }else if(encodedCurrBit + codeMap[deltaCode].encode_length > 64){
            encodedStream.push_back(encodedLatest);
            encodedLatest = (ull)codeMap[deltaCode].encode >> (64-encodedCurrBit);
            encodedCurrBit = (encodedCurrBit + codeMap[deltaCode].encode_length) % 64;
        }else{
            encodedCurrBit = encodedCurrBit + codeMap[deltaCode].encode_length;
        }
        //Argument
        if(deltaCode.ct == SpmCode::RANGE){
            ull delta = deltas[i] & ~(1ULL << ((ull)log2(deltas[i])));
            argumentLatest |= delta << argumentCurrBit;
            int width = (int)log2(deltas[i]);
            if(argumentCurrBit + width == 64){
                argumentStream.push_back(argumentLatest);
                argumentLatest = 0;
                argumentCurrBit = 0;
            }else if(argumentCurrBit + width > 64){
                argumentStream.push_back(argumentLatest);
                argumentLatest = delta >> (64-argumentCurrBit);
                argumentCurrBit = (argumentCurrBit + width) % 64;
            }else{
                argumentCurrBit += width;
            }
        }
    }
    length = encodedCurrBit;
    length += 64*encodedStream.size();
    argumentLength = argumentCurrBit;
    argumentLength += 64*argumentStream.size();

    if(encodedCurrBit != 0)
        encodedStream.push_back(encodedLatest);
    if(argumentCurrBit != 0)
        argumentStream.push_back(argumentLatest);
    encodedStream.push_back(0);
    argumentStream.push_back(0);
    double averageBits = (encodedStream.size()*64.0/deltas.size());
    ofstream log("log", ofstream::app);
    log << averageBits << endl;
    //spmCodes = codes;
    spmCodes.resize(1 << maxHuffmanLength);
    for(int i = 0; i < codes.size(); ++i){
        for(int j = codes[i].encode; j < 1 << maxHuffmanLength; j += 1 << codes[i].encode_length){
            spmCodes[j] = codes[i];
        }
    }

}

int spmCompress(SpmOptions mainOptions){
    if(mainOptions.inputFilename != "")
        freopen(mainOptions.inputFilename.c_str(), "r", stdin);
    scanf("%[^\n]", buffer);
    string matrixCode;
    bool pattern = true;
    if(string(buffer).find("real") != string::npos)
        pattern = false;
    int M, N, nnz;
    scanf("%d %d %d", &M, &N, &nnz);
    mainOptions.M = M;
    mainOptions.N = N;
    mainOptions.nnz = nnz;
    //TODO: sub row sub col
    int subRow=mainOptions.subHeight;
    int subCol=mainOptions.subWidth;
    vector<ull> row;
    vector<ull> col;
    vector<double> values;
    for(ll i = 0; i < nnz; ++i){
        ll tmp1, tmp2;
        if(!pattern){
            double tmp3;
            scanf("%lld %lld %lf", &tmp1, &tmp2, &tmp3);
        }else{
            scanf("%lld %lld", &tmp1, &tmp2);
        }
        row.push_back(tmp1-1);
        col.push_back(tmp2-1);
    }
    vector<ull> encodedStream;
    vector<ull> argumentStream;
    vector<SpmCode> codes;
    ull length, argumentLength;
    bool ret = spmCompress(row, col, codes, encodedStream, argumentStream, length, argumentLength);
    FILE* tmp;

    //freopen(tmp, "w", stdout);
    if(mainOptions.outputFilename == "")
        writeToFile(mainOptions, encodedStream, argumentStream, codes, length, argumentLength, stdout);
    else
        writeToFile(mainOptions, encodedStream, argumentStream, codes, length, argumentLength, mainOptions.outputFilename);
    return 0;
}
int extract(SpmOptions mainOptions){
    FILE* outputFile;
    if(mainOptions.outputFilename == "")
        outputFile = stdout;
    else
        outputFile = fopen(mainOptions.outputFilename.c_str(), "w");
    vector<ull> encodedStream;
    vector<ull> encodedArgumentStream;
    vector<SpmCode> codes;
    ll length;
    ll argumentLength;
    readFromFile(mainOptions, encodedStream, encodedArgumentStream, codes, length, argumentLength, mainOptions.inputFilename);
    vector<ll> decodedDeltas = decode(encodedStream, encodedArgumentStream, codes, length, argumentLength);
    //TODO:turn deltas into indices
    ll x = -1;
    ll y  = 0;
    ll newLines = 0;
    map<ll, map<ll,bool> > mapIndices;
    for(int i = 0; i < decodedDeltas.size(); ++i){
        if(decodedDeltas[i] == -1){
            newLines++;
            x = -1;
            y = newLines * mainOptions.subHeight;
        }else{
            ll leastSignificant = (decodedDeltas[i] + 1 + x % mainOptions.subWidth) % mainOptions.subWidth;
            ll mostSignificant = (decodedDeltas[i] + 1 + x % mainOptions.subWidth) / mainOptions.subWidth;
            x = (x / mainOptions.subWidth) * mainOptions.subWidth + leastSignificant;
            leastSignificant = (mostSignificant + y % mainOptions.subHeight) % mainOptions.subHeight;
            mostSignificant = (mostSignificant + y % mainOptions.subHeight) / mainOptions.subHeight;
            //TODO: finish
            y = (y / mainOptions.subHeight) * mainOptions.subHeight + leastSignificant;
            x += mostSignificant * mainOptions.subWidth;
            mapIndices[y][x] = true;
        }
    }
    fprintf(outputFile, "%%%%MatrixMarket matrix coordinate pattern general\n");
    fprintf(outputFile, "%lld %lld %lld\n", mainOptions.M, mainOptions.N, mainOptions.nnz);
    for(auto it1 = mapIndices.begin(); it1 != mapIndices.end(); ++it1)
        for(auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
            fprintf(outputFile, "%lld %lld\n", it1->first+1, it2->first+1);

    return 0;
}

struct reverseCmp {
    bool operator() (ull left, ull right) const{
        for(int i = 0; i < 64; ++i){
            if((left & (1LL << i)) < (right & (1LL << i)))
                return true;
            if((left & (1LL << i)) > (right & (1LL << i)))
                return false;
        }
        return false;
    }
};
bool readFromFile(SpmOptions &mainOptions, vector<ull> &stream, vector<ull> &argumentStream, vector<SpmCode> &codes, ll &length, ll &argumentLength, string filename){
    FILE* input = fopen(filename.c_str(), "r");
    ull codesSize;
    ull tmp;
    char* printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i){
        fscanf(input, "%c", printerPtr);
        printerPtr++;
    }
    mainOptions.M = tmp;
    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i){
        fscanf(input, "%c", printerPtr);
        printerPtr++;
    }
    mainOptions.N = tmp;
    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i){
        fscanf(input, "%c", printerPtr);
        printerPtr++;
    }
    mainOptions.nnz = tmp;
    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i){
        fscanf(input, "%c", printerPtr);
        printerPtr++;
    }
    ll codeCount = tmp;
    for(int i = 0; i < codeCount; ++i){
        SpmCode tmpCode;
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j)
            fscanf(input, "%c", printerPtr++);
        tmpCode.ct = (SpmCode::CodeType)tmp;
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j)
            fscanf(input, "%c", printerPtr++);
        tmpCode.encode = tmp;
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j)
            fscanf(input, "%c", printerPtr++);
        tmpCode.encode_length = tmp;
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j)
            fscanf(input, "%c", printerPtr++);
        tmpCode.delta = tmp;
        codes.push_back(tmpCode);
    }
    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i){
        fscanf(input, "%c", printerPtr);
        printerPtr++;
    }
    length = tmp;
    for(int i = 0; i < ((length - 1)/64 + 2); ++i){
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j){
            fscanf(input, "%c", printerPtr);
            printerPtr++;
        }
        stream.push_back(tmp);
    }

    printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i){
        fscanf(input, "%c", printerPtr);
        printerPtr++;
    }
    argumentLength = tmp;
    for(int i = 0; i < ((argumentLength - 1)/64 + 2); ++i){
        printerPtr = (char*)&tmp;
        for(int j = 0; j < 8; ++j){
            fscanf(input, "%c", printerPtr);
            printerPtr++;
        }
        argumentStream.push_back(tmp);
    }

    fclose(input);
    return true;
}
//TODO check equality
bool checkEquality(vector<ull> &streamOld, vector<ull> &streamNew, vector<SpmCode> &codesOld, vector<SpmCode> &codesNew){
    if(streamOld.size() != streamNew.size()){
        return false;
    }
    for(int i = 0; i < streamOld.size(); ++i){
        if(streamOld[i] != streamNew[i])
            return false;
    }
    if(codesOld.size() != codesNew.size())
        return false;
    for(int i = 0; i < codesOld.size(); ++i){
        if(codesOld[i].encode != codesNew[i].encode)
            return false;
        if(codesOld[i].encode_length != codesOld[i].encode_length)
            return false;
        if(codesOld[i].delta != codesOld[i].delta)
            return false;
    }
    return true;
}

vector<ll> decode(vector<ull> stream, vector<ull> argumentStream, vector<SpmCode> codes, ll length, ll argumentLength){
    int currBit = 0;
    ull oldCurrBit = 0;
    ull argumentCurrBit = 0;
    vector<ll> decoded;
    map<ull, SpmCode, reverseCmp> codesMap;
    for(int i = 0; i < codes.size(); ++i){
        codesMap[codes[i].encode] = codes[i];
    }
    //TODO: print codes
    int i = 0;
    i = 0;
    while(currBit < length) {
        ull latest = stream[currBit/64] >> (currBit % 64);
        if(currBit/64 + 1 < stream.size() && (currBit % 64) != 0)
            latest |= stream[currBit/64+1] << (64 - currBit % 64);

        auto it = codesMap.upper_bound(latest);
        if(it == codesMap.begin())
            cerr << "ERROR: bad iterator value" << endl;
        --it;
        SpmCode tmp = it->second;
        i++;
        if(tmp.encode_length == 0){
            cerr << "ERROR encode lengt equals 0" << endl;
            exit(1);
        }
        currBit += tmp.encode_length;

        if(tmp.ct == SpmCode::NEWLINE){
            decoded.push_back(-1);
        }else if(tmp.ct == SpmCode::RANGE){
            latest = argumentStream[argumentCurrBit/64] >> (argumentCurrBit % 64);
            if(argumentCurrBit/64 + 1 < argumentStream.size() && (argumentCurrBit % 64) != 0)
                latest |= argumentStream[argumentCurrBit/64+1] << (64 - argumentCurrBit % 64);
            int width = tmp.delta;
            ull mask = -1;
            if(width != 0)
                mask >>= (64-width);
            else
                mask = 0;
            ll delta = (latest & mask) | (1LL << width);
            decoded.push_back(delta);
            argumentCurrBit += width;
        }else{
            decoded.push_back(tmp.delta);
        }
        /*
        if( it->second.delta == 12){
        string user;
        cin >> user;
        */
    }
    return decoded;
}


#endif
