#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <cstdint>
#include <cstring>
#include "mmio.h"

using namespace std;
struct matrixPoint{
    uint64_t row;
    uint64_t column;
    double val;
};
//TODO: change names to is. eg. isInOrder.
//TODO: add addZeroToEmptyRows.
bool orderCheck(uint64_t* I, uint64_t* J, uint64_t nnz);
void fixOrder(uint64_t* I, uint64_t* J, double* val, uint64_t M, uint64_t N, uint64_t nnz);
bool emptyRowColCheck(uint64_t* I, uint64_t* J, uint64_t M, uint64_t N, uint64_t nnz);
void removeEmptyRowCol(uint64_t* I, uint64_t* J, uint64_t* M, uint64_t* N, uint64_t nnz);
bool nonZeroCheck(double* val, uint64_t nnz);
void removeZeros(uint64_t* I, uint64_t* J, double* val, uint64_t* nnz);
void toGrmlcm(uint64_t* I, uint64_t* J, double* val, uint64_t M, uint64_t N, uint64_t nnz, uint64_t size);
void print2dList(list<list<matrixPoint> > twoDList);
void addZerosToEmptyRows(uint64_t** I, uint64_t** J, double** val, uint64_t M, uint64_t N, uint64_t* nnz);
//TODO: void niceifyMatrix;
//TODO: read in matrix market matrix
