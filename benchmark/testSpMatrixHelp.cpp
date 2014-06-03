#include <iostream>
#include <cstdio>
#include <vector>

using namespace std;

typedef long long ll;

int main(){
    char buffer[100];
    fgets(buffer, 100, stdin);
    printf(buffer);
    ll M, N, nnz;
    scanf("lldlldlld", &M, &N, &nnz);
    vector<ll> row;
    vector<ll> column;
    vector<double> value;
    for(ll i=0;i<nnz;i++){
        ll tmp0, tmp1;
        double tmp2;
        scanf("%lld%lld%llf", &tmp0, &tmp1, &tmp2);
        row.push_back(tmp0);
        column.push_back(tmp1);
        value.push_back(tmp2);
    }
    //TODO: check order
    //TODO: fix order
    //TODO: check empty rows
    //TODO: insert zeros
}
