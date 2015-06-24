#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>

//TODO: newlines
//TODO: support very large deltas
//gamma code after greater than 64
//use 16 codes

using namespace std;
typedef long long ll;
typedef unsigned long long ull;
char buffer[200];
struct Code{
    ull encode;
    int encode_length;
    ull delta;
};

struct Node{
    int left = -1;
    int right = -1;
    int frequency;
    Code code;
};

bool compare(Node left, Node right){
    return left.frequency < right.frequency;
}

vector<Code> createCodes(vector<ll> &distribution);
ull BitsToInt(vector<bool> bits);
vector<ll> decode(vector<ull> stream, vector<Code> codes, ll length);
bool writeToFile(vector<ull> stream, vector<Code> codes, ll length, string filename);
bool readFromFile(vector<ull> &stream, vector<Code> &codes, ll &length, string filename);

int Log2(int n){
    int ret = 0;
    while(n){
        ret++;
        n>>=1;
    }
    return ret-1;
}

int main(int argc, char* argv[]){
    scanf("%[^\n]", buffer);
    cerr << buffer << endl;
    string matrixCode;
    bool pattern = true;
    if(string(buffer).find("real") != string::npos)
        pattern = false;
    int M, N, nnz;
    scanf("%d %d %d", &M, &N, &nnz);
    //TODO: sub row sub col
    int subRow=4;
    int subCol=2;
    vector<ll> row;
    vector<ll> col;
    vector<double> values;
    cerr << nnz << "\n";
    cerr << M << "\n";
    cerr << N << "\n";
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
    //rcr 42
    map<ll, map<ll, map<ll, map<ll, pair<int,int> > > > > matrix;
    for(ll i = 0; i < nnz; ++i){
        matrix[row[i]/subRow][col[i]/subCol][row[i]%subRow][col[i]%subCol] = make_pair(row[i], col[i]);
    }
    vector<ll> deltas;
    ll delta = 0;
    ll p1 = 0; ll p2 = 0; ll p3 = 0; ll p4 = 0;
    for(auto i1 = matrix.begin(); i1 != matrix.end(); ++i1){
        //delta += (i1->first - p1)*4*N; //new line
        for(int i = 0; i < i1->first - p1; ++i)
            deltas.push_back(-1);
        for(auto i2 = i1->second.begin(); i2 != i1->second.end(); ++i2){
            delta += (i2->first - p2)*subRow*subCol;
            for(auto i3 = i2->second.begin(); i3 != i2->second.end(); ++i3){
                delta += (i3->first - p3)*subCol;
                for(auto i4 = i3->second.begin(); i4 != i3->second.end(); ++i4){
                    delta += i4->first - p4;
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
    vector<ll> distribution;
    int huffmanCodesSize = 6;
    distribution.resize(huffmanCodesSize);

    for(int i = 0; i < deltas.size(); ++i){
        if(deltas[i] == -1)
            distribution[huffmanCodesSize-2]++;
        else if(deltas[i] < huffmanCodesSize-2)
            distribution[deltas[i]]++;
        else
            distribution[huffmanCodesSize-1]++;
    }

    vector<Code> codes = createCodes(distribution);
    map<ll, Code> codeMap;
    for(int i = 0; i < codes.size(); ++i){
        if(codes[i].delta == huffmanCodesSize-2){
            codes[i].delta = -1;
            codeMap[-1] = codes[i];
    }else
        codeMap[codes[i].delta] = codes[i];
    }
    vector<ull> encodedStream;
    ll currBit = 0;
    ll latest = 0;
    for(ll i = 0; i < deltas.size(); ++i){
        int delta = deltas[i];
        if(delta >= huffmanCodesSize-2)
            delta = huffmanCodesSize-1;
        latest |= codeMap[delta].encode << currBit;
        if(currBit + codeMap[delta].encode_length == 64){
            encodedStream.push_back(latest);
            latest = 0;
            currBit = 0;
        }else if(currBit + codeMap[delta].encode_length > 64){
            encodedStream.push_back(latest);
            latest = codeMap[delta].encode >> (64-currBit);
            currBit = (currBit + codeMap[delta].encode_length) % 64;
        }else{
            currBit = currBit + codeMap[delta].encode_length;
        }
        //TODO: gamma code
        if(delta == huffmanCodesSize-1){
            int zerosNeeded = Log2(deltas[i]) - Log2(huffmanCodesSize-2);
            if(currBit + zerosNeeded >= 64){
                encodedStream.push_back(latest);
                latest=0;
            }
            currBit = (currBit + zerosNeeded) % 64;
            latest |= 1ULL << currBit;
            currBit++;
            if(currBit == 64){
                encodedStream.push_back(latest);
                latest=0;
                currBit = 0;
            }

            ull delta = deltas[i] & ~(1ULL << (Log2(deltas[i])));
            latest |= delta << currBit;
            int width = Log2(deltas[i]);
            if(currBit + width == 64){
                encodedStream.push_back(latest);
                latest = 0;
                currBit = 0;
            }else if(currBit + width > 64){
                encodedStream.push_back(latest);
                latest = delta >> (64-currBit);
                currBit = (currBit + width) % 64;
            }else{
                currBit += width;
            }
            //TODO: bits except first
        }
    }
    if(currBit != 0)
        encodedStream.push_back(latest);
    cerr << "Size (bytes): " << (encodedStream.size()*8) << endl;
    cerr << "Bits per index: " << (encodedStream.size()*64.0/deltas.size()) << endl;
    double averageBits = (encodedStream.size()*64.0/deltas.size());
    ofstream log("log", ofstream::app);
    log << averageBits << endl;


    //decoding
    int length = currBit;
    if(currBit == 0)
        length += 64*encodedStream.size();
    else
        length += 64*(encodedStream.size() - 1);
    writeToFile(encodedStream, codes, length, "output.rcr");
    vector<ull> reencodedStream;
    vector<Code> recodes;
    ll relength;
    readFromFile(reencodedStream, recodes, relength, "output.rcr");

    vector<ll> decodedDeltas = decode(encodedStream, codes, length);
    if(false){
        for(int i = 0; i < decodedDeltas.size(); ++i){
            cerr << dec << decodedDeltas[i] << " ";
            if(decodedDeltas[i] != deltas[i]){
                cerr << endl;
                cerr << "ERROR: missmatch" << endl;
                cerr << "index: " << i << endl;
                cerr << "real: " << deltas[i] << endl;
                cerr << "decoded: " << decodedDeltas[i] << endl;
                return 1;
            }
        }
    }
    //TODO: deltas to indices
    //TODO: check
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
bool writeToFile(vector<ull> stream, vector<Code> codes, ll length, string filename){
    FILE* output = fopen(filename.c_str(),"w");
    //TODO: print rcr file
    //number of codes
    ull tmp = codes.size();
    char* printerPtr = (char*)&tmp;
    cerr << "output codes size: " << (*printerPtr) << endl;
    for(int i = 0; i < 8; ++i){
        fprintf(output, "%c", printerPtr[i]);
    }
    for(int i = 0; i < codes.size(); ++i){
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
    //stream
    fclose(output);
    return true;
}
bool readFromFile(vector<ull> &stream, vector<Code> &codes, ll &length, string filename){
    FILE* input = fopen(filename.c_str(), "r");
    ull codesSize;
    ull tmp;
    char* printerPtr = (char*)&tmp;
    for(int i = 0; i < 8; ++i){
        fscanf(input, "%c", printerPtr);
        cerr << ((int)*printerPtr) << endl;
        printerPtr++;
    }
    cerr << "read codes size: " << tmp << endl;
    ll codeCount = tmp;
    for(int i = 0; i < tmp; ++i){
        for(
    }
    fclose(input);
    return true;
}

vector<ll> decode(vector<ull> stream, vector<Code> codes, ll length){
    int currBit = 0;
    vector<ll> decoded;
    map<ull, Code, reverseCmp> codesMap;
    for(int i = 0; i < codes.size(); ++i){
        codesMap[codes[i].encode] = codes[i];
    }
    //TODO: print codes
    int i = 0;
//    for(auto it = codesMap.begin(); it != codesMap.end(); ++it){
//        cerr << dec << "code index: " << i++ << endl;
//        cerr << hex << "encode: " << it->first << endl;
//        cerr << dec << "delta: " << it->second.delta << endl;
//    }
    while(currBit < length) {
        ull latest = stream[currBit/64] >> (currBit % 64);
        if(currBit/64 + 1 < stream.size() && (currBit % 64) != 0)
            latest |= stream[currBit/64+1] << (64 - currBit % 64);

//        cerr << "decode: \n";
//        cerr << hex << latest << endl;
//        if(codesMap.lower_bound(latest) == codesMap.end())
//            cerr << "wtf" << endl;
//        cerr << hex << codesMap.lower_bound(latest)->first << endl;
        //cerr << hex << ((codesMap.lower_bound(latest))-1)->first << endl;
        auto it = codesMap.upper_bound(latest);
        --it;
        Code tmp = it->second;
        //cerr << "delta: " << it->second.delta << endl;
        currBit += tmp.encode_length;
        //TODO: decode gamma code
        if(tmp.delta == codes.size() - 1){
            latest = stream[currBit/64] >> (currBit % 64);
            if(currBit/64 + 1 < stream.size() && (currBit % 64) != 0)
                latest |= stream[currBit/64+1] << (64 - currBit % 64);
            int width = 0;
            while(!(latest & 1)){
                currBit++;
                latest >>= 1;
                width++;
            }
            width += Log2(codes.size()-2);
            currBit++;
            latest >>= 1;
            ull mask = -1;
            if(width != 0)
                mask >>= (64-width);
            else
                mask = 0;
            ll delta = (latest & mask) | (1LL << width);
            decoded.push_back(delta);
            currBit += width;
        }else{
            decoded.push_back(tmp.delta);
        }
    }
    return decoded;
}

vector<Code> createCodes(vector<ll> &distribution){
    vector<Code> codes;
    vector<Node> tree;
    for(int i = 0; i < distribution.size(); ++i){
        Node tmp;
        tmp.frequency = distribution[i];
        tmp.code.delta = i;
        tree.push_back(tmp);
    }
    for(int i = 0; i < tree.size()-1; i += 2){
        sort(tree.begin()+i,tree.end(),compare);
        Node tmp;
        tmp.left = i;
        tmp.right = i+1;
        tmp.frequency = tree[i].frequency + tree[i+1].frequency;
        tree.push_back(tmp);
    }
//    cerr << "test\n";
//    for(int i = 0; i < tree.size(); ++i){
//        cerr << tree[i].frequency << endl;
//    }

    //TODO: create codes
    vector<bool> path;
    vector<int> parentStack;
    vector<int> phase;
    int curr = tree.size() - 1;
    phase.push_back(0);
    while(phase[0] != 2 || phase.size() != 1){
//        cerr << "dfs: " << curr << endl;
//        cerr << "children: " << tree[curr].left << " and " << tree[curr].right << endl;
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
                    Code tmp = tree[curr].code;
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
//    cerr << "codes: " << codes.size() << endl;
//    for(int i = 0; i < codes.size(); ++i){
//        cerr << "index: " << i << ":\n";
//        cerr << "encode: " << codes[i].encode << endl;
//        cerr << "encode length: " << codes[i].encode_length << endl;
//        cerr << "delta: " << codes[i].delta << endl;
//    }
    return codes;
}

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
