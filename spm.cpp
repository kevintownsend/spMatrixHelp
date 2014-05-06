#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <stdint.h>
#include <string.h>
#include "mmio.h"

using namespace std;
struct matrixPoint{
    uint64_t row;
    uint64_t column;
    double val;
};
bool orderCheck(uint64_t* I, uint64_t* J, uint64_t nnz);
void fixOrder(uint64_t* I, uint64_t* J, double* val, uint64_t M, uint64_t N, uint64_t nnz);
bool emptyRowColCheck(uint64_t* I, uint64_t* J, uint64_t M, uint64_t N, uint64_t nnz);
void removeEmptyRowCol(uint64_t* I, uint64_t* J, uint64_t* M, uint64_t* N, uint64_t nnz);
bool nonZeroCheck(double* val, uint64_t nnz);
void removeZeros(uint64_t* I, uint64_t* J, double* val, uint64_t* nnz);
void toGrmlcm(uint64_t* I, uint64_t* J, double* val, uint64_t M, uint64_t N, uint64_t nnz, uint64_t size);
void print2dList(list<list<matrixPoint> > twoDList);

int main(int argc, char* argv[]){
    if(argc < 1){
        cerr << "usage: " << argv[0] << " < input.mtx > output.mtx" << endl;
        return 0;
    }
    uint64_t GRMLCM=0;
    uint64_t rcrRowSize=0;
    uint64_t rcrColSize=0;
    for(int i=1; i < argc; i++){
        string argString = string(argv[i]);
        if(argString[0] == '-'){
            if(argString.substr(1,6) == "GRMLCM"){
                GRMLCM = atoi(argString.substr(7, string::npos).c_str());
            }else if(argString.substr(1,3) == "RCR"){
                string tmp = argString.substr(4, string::npos);
                int spliter = tmp.find('x'); //TODO: error check
                rcrRowSize = atoi(tmp.substr(0,spliter-1).c_str());
                rcrColSize = atoi(tmp.substr(spliter+1,string::npos).c_str());
            }else{
                cerr << "usage: " << argv[0] << " < input.mtx > output.mtx" << endl;
                return 0;
            }
        }else{
            cerr << "usage: " << argv[0] << " < input.mtx > output.mtx" << endl;
            return 0;
            
        }
        
    }
    cerr << "GRMLCM:" << GRMLCM << endl;
    //TODO: get unsymetric working
    //TODO: then get symetric
    //TODO: test empty rows and columns
    //TODO: test for out of order
    bool symmetric = false;
    bool general = false;
    bool real = false;
    bool pattern = false;
    string tmpLine;
    getline(cin, tmpLine);
    if(tmpLine == "\%\%MatrixMarket matrix coordinate real general"){
        real = true;
        general = true;
        cout <<  "\%\%MatrixMarket matrix coordinate real general" << endl;
    }else if(tmpLine == "\%\%MatrixMarket matrix coordinate real symmetric"){
        cout << "\%\%MatrixMarket matrix coordinate real general" << endl;
        real = true;
        symmetric = true;
    }else if(tmpLine == "\%\%MatrixMarket matrix coordinate pattern general"){
        cout << "\%\%MatrixMarket matrix coordinate real general" << endl;
        pattern = true;
        general = true;
    }else{
        cerr << "Unsupported format" << endl;
        return 0;
    }
    getline(cin, tmpLine);
    while(tmpLine[0] == '%')
        getline(cin, tmpLine);
    uint64_t M, N, nnz;
    sscanf(tmpLine.c_str(), "%ld %ld %ld", &M, &N, &nnz);
    uint64_t *I;
    uint64_t *J;
    double *val;
    
    if(symmetric){
        I = (uint64_t*)malloc(2 * nnz * sizeof(uint64_t));
        J = (uint64_t*)malloc(2 * nnz * sizeof(uint64_t));
        val = (double*)malloc(2 * nnz * sizeof(uint64_t));
    }else{
        I = (uint64_t*)malloc(nnz * sizeof(uint64_t));
        J = (uint64_t*)malloc(nnz * sizeof(uint64_t));
        val = (double*)malloc(nnz * sizeof(uint64_t));
    }
    uint64_t tmpI, tmpJ;
    double tmpVal;
    if(pattern){
        for(uint64_t i = 0; i < nnz; i++){
            getline(cin, tmpLine);
            sscanf(tmpLine.c_str(), "%ld %ld", &tmpI, &tmpJ);
            I[i] = tmpI - 1;
            J[i] = tmpJ - 1;
            val[i] = 1.0;
        }
    }else if(symmetric){
        uint64_t tmpNnz = nnz;
        uint64_t i2 = 0;
        for(uint64_t i = 0; i < nnz; i++, i2++){
            getline(cin, tmpLine);
            sscanf(tmpLine.c_str(), "%ld %ld %lf", &tmpI, &tmpJ, &tmpVal);
            I[i2] = tmpI - 1;
            J[i2] = tmpJ - 1;
            val[i2] = tmpVal;
            if(tmpI != tmpJ){
                i2++;
                I[i2] = tmpJ - 1;
                J[i2] = tmpI - 1;
                val[i2] = tmpVal;
            }
        }
        nnz = i2;
    }else{
        for(uint64_t i = 0; i < nnz; i++){
            getline(cin, tmpLine);
            sscanf(tmpLine.c_str(), "%ld %ld %lf", &tmpI, &tmpJ, &tmpVal);
            I[i] = tmpI - 1;
            J[i] = tmpJ - 1;
            val[i] = tmpVal;
        }
    }
    if(nonZeroCheck(val, nnz))
        cerr << "no non zeros" << endl;
    else{
        cerr << "Warning: explicit non-zeros" << endl;
        removeZeros(I, J, val, &nnz);
    }
    if(orderCheck((uint64_t*)I,(uint64_t*)J,nnz))
        cerr << "In order" << endl;
    else{
        if(symmetric)
            cerr << "Out of Order, symmetric" << endl;
        else
            cerr << "Out of Order" << endl;
        cerr << "beginning fix order" << endl;
        fixOrder(I, J, val, M, N, nnz);
        cerr << "end fix order" << endl;
    }
    if(emptyRowColCheck(I, J, M, N, nnz))
        cerr << "No empty Rows or Columns" << endl;
    else{
        cerr << "Warning: empty Rows or Columns" << endl;
        removeEmptyRowCol(I, J, &M, &N, nnz);
    }
    
    if(orderCheck((uint64_t*)I,(uint64_t*)J,nnz))
        cerr << "In order" << endl;
    if(emptyRowColCheck(I, J, M, N, nnz))
        cerr << "No empty Rows or Columns" << endl;
    if(nonZeroCheck(val,nnz))
        cerr << "No explicit non-zeros" << endl;
    //TODO: GRMLCM traversal
    if(GRMLCM > 0){
        toGrmlcm(I, J, val, M, N, nnz, GRMLCM);
    }
    cout << M << " " << N << " " << nnz << endl;
    for(uint64_t i = 0; i < nnz; i++){
        cout << (I[i] + 1) << " " << (J[i] + 1) << " " << val[i] << endl;
    }
    free(J);
    free(I);
    free(val);
    return 0;
}

bool nonZeroCheck(double* val, uint64_t nnz){
    for(uint64_t i = 0; i < nnz; i++){
        if(val[i] == 0)
            return false;
    }
    return true;
}

void removeZeros(uint64_t* I, uint64_t* J, double* val, uint64_t* nnz){
    uint64_t zeroCount = 0;
    for(uint64_t i = 0; i < *nnz; i++){
        if(val[i] == 0){
            zeroCount++;
        }else{
            uint64_t tmp = i - zeroCount;
            val[tmp] = val[i];
            I[tmp] = I[i];
            J[tmp] = J[i];
        }
    }
    *nnz = *nnz - zeroCount;
}

bool orderCheck(uint64_t* I, uint64_t* J, uint64_t nnz){
    for(uint64_t i = 1; i < nnz; i++){
        if(I[i] < I[i - 1])
            return false;
        if((J[i] < J[i - 1]) && (I[i] == I[i - 1]))
            return false;
    }
    return true;
}

void fixOrder(uint64_t* I, uint64_t* J, double* val, uint64_t M, uint64_t N, uint64_t nnz){
    cerr << "I: " << I[0] << " J: " << J[0] << " val: " << val[0] << " M: " << M << " N: " << N << " nnz: " << nnz << endl;
    vector< list<uint64_t> > listJ;
    vector< list<double> > listVal;
    listJ.resize(M);
    listVal.resize(M);
    for(uint64_t i = 0; i < nnz; i++){
        /*if(i%1000000 == 0)
            cerr << "current: " << i << endl;*/
        uint64_t currI = I[i];
        uint64_t currJ = J[i];
        list<uint64_t>::iterator itEnd = listJ[currI].end();
        list<uint64_t>::iterator itI = listJ[currI].begin();
        list<double>::iterator itVal = listVal[currI].begin();
        while(itI != itEnd){
            if(currJ > *itI){
                itI++;
                itVal++;
            }else{
                break;
            }
        }
        listJ[currI].insert(itI,currJ);
        listVal[currI].insert(itVal,val[i]);
        
    }
    cerr << "structure created" << endl;
    uint64_t traverseIndex = 0;
    for(uint64_t i = 0; i < M; i++){
        list<double>::iterator itVal = listVal[i].begin();
        for(list<uint64_t>::iterator it = listJ[i].begin(); it != listJ[i].end(); it++){
            I[traverseIndex] = i;
            J[traverseIndex] = *it;
            val[traverseIndex] = *itVal;
            itVal++;
            traverseIndex++;
        }
    }
}

void toRowColRow(uint64_t* I, uint64_t* J, double* val, uint64_t M, uint64_t N, uint64_t nnz, uint64_t rowSize, uint64_t colSize){
    list< list<matrixPoint> > lColRow;
    uint64_t currentGroup = 0;
    uint64_t groupChanges = 0;
    uint64_t groupStart = 0;
    for(uint64_t currentVal = 0; currentVal < nnz; currentVal++){
        //print2dList(lColRow);
        bool groupDone;
        groupDone = currentGroup + rowSize <= I[currentVal];
        if(groupDone){
            for(list<list<matrixPoint> >::iterator itI = lColRow.begin(); itI != lColRow.end(); itI++){
                for(list<matrixPoint>::iterator itJ = (*itI).begin(); itJ != (*itI).end(); itJ++){
                    I[groupStart] = (*itJ).row;
                    J[groupStart] = (*itJ).column;
                    val[groupStart] = (*itJ).val;
                    groupStart++;
                }
            }
            currentGroup += rowSize;
            groupChanges++;
            lColRow.clear();
        }else{
        }
        list<list<matrixPoint> >::iterator itI = lColRow.begin();
        list<list<matrixPoint> >::iterator itIend = lColRow.end();
        while(true){
            if(itI == itIend){
                lColRow.insert(itI, list<matrixPoint>(1,matrixPoint()));
                (*((*--itI).begin())).row = I[currentVal];
                (*((*itI).begin())).column = J[currentVal];
                (*((*itI).begin())).val = val[currentVal];
                break;
            }else if((*itI).front().column > J[currentVal]){
                lColRow.insert(itI, list<matrixPoint>(1,matrixPoint()));
                (*((*--itI).begin())).row = I[currentVal];
                (*((*itI).begin())).column = J[currentVal];
                (*((*itI).begin())).val = val[currentVal];
                break;
            }else if((*itI).front().column == J[currentVal]){
                list<matrixPoint>::iterator itJ = (*itI).begin();
                list<matrixPoint>::iterator itJend = (*itI).end();
                int errCheck = 0;
                while(true){
                    if(itJ == itJend){
                        (*itI).insert(itJ, matrixPoint());
                        (*--itJ).row = I[currentVal];
                        (*itJ).column = J[currentVal];
                        (*itJ).val = val[currentVal];
                        break;
                    }else if((*itJ).row > I[currentVal]){
                        (*itI).insert(itJ, matrixPoint());
                        (*--itJ).row = I[currentVal];
                        (*itJ).column = J[currentVal];
                        (*itJ).val = val[currentVal];
                        break;
                    }else if((*itJ).row == I[currentVal]){
                        cerr << "ERROR: 2 values in same place" << endl;
                        return;
                        break;
                    }
                    itJ++;

                }
                break;
            }
            itI++;
        }
        list<matrixPoint>::iterator itJ = (*itI).begin();
    }
    for(list<list<matrixPoint> >::iterator itI = lColRow.begin(); itI != lColRow.end(); itI++){
        for(list<matrixPoint>::iterator itJ = (*itI).begin(); itJ != (*itI).end(); itJ++){
            I[groupStart] = (*itJ).row;
            J[groupStart] = (*itJ).column;
            val[groupStart] = (*itJ).val;
            groupStart++;
        }
    }
    cerr << "Group changes:" << groupChanges << endl;
}

void toGrmlcm(uint64_t* I, uint64_t* J, double* val, uint64_t M, uint64_t N, uint64_t nnz, uint64_t size){
    list< list<matrixPoint> > lColRow;
    uint64_t currentGroup = 0;
    uint64_t groupChanges = 0;
    uint64_t groupStart = 0;
    for(uint64_t currentVal = 0; currentVal < nnz; currentVal++){
        //print2dList(lColRow);
        bool groupDone;
        groupDone = currentGroup + size <= I[currentVal];
        if(groupDone){
            for(list<list<matrixPoint> >::iterator itI = lColRow.begin(); itI != lColRow.end(); itI++){
                for(list<matrixPoint>::iterator itJ = (*itI).begin(); itJ != (*itI).end(); itJ++){
                    I[groupStart] = (*itJ).row;
                    J[groupStart] = (*itJ).column;
                    val[groupStart] = (*itJ).val;
                    groupStart++;
                }
            }
            currentGroup += size;
            groupChanges++;
            lColRow.clear();
        }else{
        }
        list<list<matrixPoint> >::iterator itI = lColRow.begin();
        list<list<matrixPoint> >::iterator itIend = lColRow.end();
        while(true){
            if(itI == itIend){
                lColRow.insert(itI, list<matrixPoint>(1,matrixPoint()));
                (*((*--itI).begin())).row = I[currentVal];
                (*((*itI).begin())).column = J[currentVal];
                (*((*itI).begin())).val = val[currentVal];
                break;
            }else if((*itI).front().column > J[currentVal]){
                lColRow.insert(itI, list<matrixPoint>(1,matrixPoint()));
                (*((*--itI).begin())).row = I[currentVal];
                (*((*itI).begin())).column = J[currentVal];
                (*((*itI).begin())).val = val[currentVal];
                break;
            }else if((*itI).front().column == J[currentVal]){
                list<matrixPoint>::iterator itJ = (*itI).begin();
                list<matrixPoint>::iterator itJend = (*itI).end();
                int errCheck = 0;
                while(true){
                    if(itJ == itJend){
                        (*itI).insert(itJ, matrixPoint());
                        (*--itJ).row = I[currentVal];
                        (*itJ).column = J[currentVal];
                        (*itJ).val = val[currentVal];
                        break;
                    }else if((*itJ).row > I[currentVal]){
                        (*itI).insert(itJ, matrixPoint());
                        (*--itJ).row = I[currentVal];
                        (*itJ).column = J[currentVal];
                        (*itJ).val = val[currentVal];
                        break;
                    }else if((*itJ).row == I[currentVal]){
                        cerr << "ERROR: 2 values in same place" << endl;
                        return;
                        break;
                    }
                    itJ++;

                }
                break;
            }
            itI++;
        }
        list<matrixPoint>::iterator itJ = (*itI).begin();
    }
    for(list<list<matrixPoint> >::iterator itI = lColRow.begin(); itI != lColRow.end(); itI++){
        for(list<matrixPoint>::iterator itJ = (*itI).begin(); itJ != (*itI).end(); itJ++){
            I[groupStart] = (*itJ).row;
            J[groupStart] = (*itJ).column;
            val[groupStart] = (*itJ).val;
            groupStart++;
        }
    }
    cerr << "Group changes:" << groupChanges << endl;
}
void print2dList(list<list<matrixPoint> > twoDList){
    cerr << "Current List:" << endl;
    for(list<list<matrixPoint> >::iterator itI = twoDList.begin(); itI != twoDList.end(); itI++){
        for(list<matrixPoint>::iterator itJ = (*itI).begin(); itJ != (*itI).end(); itJ++){
            cerr << (*itJ).row << "," << (*itJ).column << ":" << (*itJ).val << endl;
        }
        cerr << "new column" << endl;
    }
}

bool emptyRowColCheck(uint64_t* I, uint64_t* J, uint64_t M, uint64_t N, uint64_t nnz){
    bool* rowHasElements = (bool*)malloc(M * sizeof(bool));
    bool* colHasElements = (bool*)malloc(N * sizeof(bool));
    bool ret = true;
    for(uint64_t i = 0; i < M; i++){
        rowHasElements[i] = false;
    }
    for(uint64_t i = 0; i < N; i++){
        colHasElements[i] = false;
    }
    for(uint64_t i = 0; i < nnz; i++){
        rowHasElements[I[i]] = true;
        colHasElements[J[i]] = true;
    }
    for(uint64_t i = 0; i < M; i++){
        if(!rowHasElements[i])
            ret = false;
    }
    for(uint64_t i = 0; i < N; i++){
        if(!colHasElements[i])
            ret = false;
    }
    free(rowHasElements);
    free(colHasElements);
    return ret;
}

void removeEmptyRowCol(uint64_t* I, uint64_t* J, uint64_t* M, uint64_t* N, uint64_t nnz){
    bool* rowHasElements = (bool*)malloc(*M * sizeof(bool));
    bool* colHasElements = (bool*)malloc(*N * sizeof(bool));
    bool ret = true;
    for(uint64_t i = 0; i < *M; i++){
        rowHasElements[i] = false;
    }
    for(uint64_t i = 0; i < *N; i++){
        colHasElements[i] = false;
    }
    for(uint64_t i = 0; i < nnz; i++){
        rowHasElements[I[i]] = true;
        colHasElements[J[i]] = true;
    }
    for(uint64_t i = 0; i < *M; i++){
        if(!rowHasElements[i])
            ret = false;
    }
    for(uint64_t i = 0; i < *N; i++){
        if(!colHasElements[i])
            ret = false;
    }
    uint64_t* cummEmptyRow = (uint64_t*)malloc(*M * sizeof(uint64_t));
    uint64_t* cummEmptyCol = (uint64_t*)malloc(*N * sizeof(uint64_t));
    uint64_t emptyCountI, emptyCountJ;
    cummEmptyRow[0] = !rowHasElements[0];
    for(uint64_t i = 1; i < *M; i++)
        cummEmptyRow[i] = cummEmptyRow[i - 1] + !rowHasElements[i];
    cummEmptyCol[0] = !colHasElements[0];
    for(uint64_t i = 1; i < *N; i++)
        cummEmptyCol[i] = cummEmptyCol[i - 1] + !colHasElements[i];
    for(uint64_t i = 0; i < nnz; i++){
        I[i] = I[i] - cummEmptyRow[I[i]];
        J[i] = J[i] - cummEmptyCol[J[i]];
    }
    *M = *M - cummEmptyRow[*M - 1];
    *N = *N - cummEmptyCol[*N - 1];

    free(cummEmptyCol);
    free(cummEmptyRow);
    free(rowHasElements);
    free(colHasElements);
}
