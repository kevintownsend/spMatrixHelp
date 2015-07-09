#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>

using namespace std;

typedef long long ll;
typedef unsigned long long ull;
char buffer[200];
int main(int argc, char* argv[]){
    scanf("%[^\n]", buffer);
    string firstLine(buffer);
    int M, N, nnz;
    scanf("%d %d %d", &M, &N, &nnz);
    bool booleanValues = false;
    if(firstLine.find("pattern") != string::npos)
        booleanValues = true;
    map<ll, map<ll,double> > mapMatrix;
    for(int i = 0; i < nnz; ++i){
        ll tmp1, tmp2;
        double tmp3 = 1.0;
        if(booleanValues)
            scanf("%lld %lld", &tmp1, &tmp2);
        else
            scanf("%lld %lld %lf", &tmp1, &tmp2, &tmp3);
        mapMatrix[tmp1][tmp2] = tmp3;
    }
    if(!booleanValues){
        firstLine.replace(firstLine.find("real"), 4, "pattern");
    }
    printf("%s\n", firstLine.c_str());
    printf("%d %d %d\n", M, N, nnz);
    for(auto it1 = mapMatrix.begin(); it1 != mapMatrix.end(); ++it1)
    for(auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
        printf("%lld %lld\n", it1->first, it2->first);
}
