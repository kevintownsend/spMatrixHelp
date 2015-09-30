#ifndef RCR_HELPER_HPP
#define RCR_HELPER_HPP

#include "rcr.hpp"


using namespace std;
typedef long long ll;
typedef unsigned long long ull;
vector<Code> createCodes(map<Code, ll> &distribution, ll nnz = -1, ll maxLength = 32);
ull BitsToInt(vector<bool> bits);
vector<ll> decode(vector<ull> stream, vector<ull> argumentStream, vector<Code> codes, ll length, ll argumentLength);
bool writeToFile(Options &mainOptions, vector<ull> &stream, vector<ull> &argumentStream, vector<Code> &codes, ll length, ll argumentLength, string filename);
bool writeToFile(Options &mainOptions, vector<ull> &stream, vector<ull> &argumentStream, vector<Code> &codes, ll length, ll argumentLength, FILE* output);
bool readFromFile(Options &mainOptions, vector<ull> &stream, vector<ull> &argumentStream, vector<Code> &codes, ll &length, ll &argumentLength, string filename);
bool checkEquality(vector<ull> &streamOld, vector<ull> &streamNew, vector<Code> &codesOld, vector<Code> &codesNew);

int compress(Options mainOptions);
int extract(Options mainOptions);
int FindMaxCodeLength(vector<Code> codes);

#endif
