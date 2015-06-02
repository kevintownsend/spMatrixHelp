#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;
typedef long long ll;
char buffer[200];
struct Code{
    ll encode;
    int encode_length;
    ll delta;
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
ll BitsToInt(vector<bool> bits);

int main(int argc, char* argv[]){
    scanf("%[^\n]", buffer);
    cerr << buffer << endl;
    int M, N, nnz;
    scanf("%d %d %d", &M, &N, &nnz);
    vector<ll> row;
    vector<ll> col;
    vector<double> values;
    cerr << nnz << "\n";
    cerr << M << "\n";
    cerr << N << "\n";
    for(ll i = 0; i < nnz; ++i){
        ll tmp1, tmp2;
        scanf("%ld %ld", &tmp1, &tmp2);
        row.push_back(tmp1-1);
        col.push_back(tmp2-1);
    }
    //rcr 42
    map<ll, map<ll, map<ll, map<ll, double> > > > matrix;
    for(ll i = 0; i < nnz; ++i){
        matrix[row[i]/4][col[i]/2][row[i]%4][col[i]%2] = 1;
    }
    vector<ll> deltas;
    ll delta = -1;
    ll p1 = 0; ll p2 = 0; ll p3 = 0; ll p4 = 0;
    for(auto i1 = matrix.begin(); i1 != matrix.end(); ++i1){
        delta += (i1->first - p1)*4*N; //new line
        for(auto i2 = i1->second.begin(); i2 != i1->second.end(); ++i2){
            delta += (i2->first - p2)*4*2;
            for(auto i3 = i2->second.begin(); i3 != i2->second.end(); ++i3){
                delta += (i3->first - p3)*2;
                for(auto i4 = i3->second.begin(); i4 != i3->second.end(); ++i4){
                    delta += i4->first - p4;
                    deltas.push_back(delta+1);
                    delta = 0;
                    p4 = i4->first;
                }
                p4 = i3->first;
            }
            p2 = i2->first;
        }
        p1 = i1->first;
    }
    vector<ll> distribution;
    distribution.resize(64);

    for(int i = 0; i < deltas.size(); ++i){
        if(deltas[i] < 64)
            distribution[deltas[i]]++;
        else
            distribution[63]++;
        cerr << deltas[i] << endl;
    }

    cerr << "distribution:" << endl;
    for(int i = 0; i < 64; ++i){
        cerr << distribution[i] << endl;
    }
    vector<Code> codes = createCodes(distribution);
    map<ll, Code> codeMap;
    for(int i = 0; i < codes.size(); ++i){
        codeMap[codes[i].delta] = codes[i];
    }
    vector<ll> encodedStream;
    ll currBit = 0;
    ll latest = 0;
    for(ll i = 0; i < deltas.size(); ++i){
        latest |= codeMap[deltas[i]].encode << currBit;
        if(currBit +codeMap[deltas[i]].encode_length == 64){
            encodedStream.push_back(latest);
            latest = 0;
            currBit = 0;
        }else if(currBit + codeMap[deltas[i]].encode_length > 64){
            encodedStream.push_back(latest);
            latest = codeMap[deltas[i]].encode >> (64-currBit);
            currBit = (currBit + codeMap[deltas[i]].encode_length) % 64;
        }else{
            currBit = currBit + codeMap[deltas[i]].encode_length;
        }
    }
    if(currBit != 0)
        encodedStream.push_back(latest);
    //TODO: decoding

}

vector<ll> decode(vector<ll> stream, vector<Code> codes, ll length){
    int currBit = 0;
    vector<ll> decoded;
    map<ll, Code> codesMap;
    for(int i = 0; i < codes.size(); ++i){
        codesMap[codes[i].encode] = codes[i];
    }
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
    cerr << "test\n";
    for(int i = 0; i < tree.size(); ++i){
        cerr << tree[i].frequency << endl;
    }

    //TODO: create codes
    vector<bool> path;
    vector<int> parentStack;
    vector<int> phase;
    int curr = tree.size() - 1;
    phase.push_back(0);
    while(phase[0] != 2){
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
                phase.pop_back();
                curr=parentStack.back();
                parentStack.pop_back();
                path.pop_back();
                if(tree[curr].left == -1 && tree[curr].right == -1){
                    Code tmp = tree[curr].code;
                    tmp.encode = BitsToInt(path);
                    tmp.encode_length = path.size();
                    codes.push_back(tmp);
                }
                break;
            default:
                break;
        }
    }
    return codes;
}

ll BitsToInt(vector<bool> bits){
    ll ret = 0;
    if(bits.size() > 64)
        cerr << "ERROR: too many bits" << endl;
    for(int i = 0; i < bits.size(); ++i){
        if(bits[i])
            ret |= 1 << i;
    }
    return ret;
}
