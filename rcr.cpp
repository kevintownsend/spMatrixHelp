#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
using namespace std;
typedef long long ll;
char buffer[200];
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

    //TODO:read file
    //TODO: traversal
    //TODO: encoding
}

struct Code{
    ll encode;
    int encode_length;
    ll delta;
};

struct node{
    node* left = nullptr;
    node* right = nullptr;
    int frequency;
    Code code;
};

vector<Code> createCodes(){
    vector<Code> codes;
    vector<Node> tree;

    return codes;
}
