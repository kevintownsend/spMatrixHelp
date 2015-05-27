#include <ios>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <stdint.h>
#include <sys/stat.h>
#include "mmio.h"
#include "mcv.h"

#define COMPRESS 0x01
#define COO 0x02
#define CSR 0x04
#define DELTA 0x08
#define MCV256 0x10
//TODO: 256 mcv


using namespace std;
void usage(char*);
uint8_t bitLength(uint64_t value);
//TODO: description
int main(int argc, char* argv[]){
    //TODO: help
    //TODO: error checking
    //TODO: decode and encode
    //TODO: short name sparse matrix compressor smc SMaC
    //TODO: option "--" for long names "-" for short eg d 
    // options: --coo -o --csr -w --extract -x --compress -c --r3format -r --fpc -f --delta -e --diagonal -i --ellpack -l --hybrid -h --packet -p --single -s --double -d --half -h
    // cont. --quad -q --bigMatrices -b --GRMLCM# -g# --mcv256
    //TODO: Coiling effect
    char buffer[100];
    if(argc == 1){
        cout << "Usage: " << argv[0] << " options source destDir" << endl;
        return 0;
    }
    if(string(argv[1]) == "-h"){
        cout << "Yeah... e-mail kt@krtownsend.com" << endl;
        return 0;
    }
    int argIndex = 1;
    if(argv[1] == NULL){
        usage(argv[0]);
        return 0;
    }
    ofstream logFile("smac.log");
    logFile << "Hello World" << endl;
    logFile.close();
    uint8_t mode = COMPRESS|COO;
    uint64_t GRMLCM = 0;
    while(string(argv[argIndex])[0] == '-'){
        if(argv[argIndex][1] == '-'){
            string input(&argv[argIndex][2]);
            if(input == "coo"){
                mode |= COO;
            }else if(input == "csr"){
                mode |= CSR;
                mode &= ~COO;
            }else if(input == "create"){
                mode |= COMPRESS;
            }else if(input == "extract"){
                mode &= ~COMPRESS;
            }else if(input == "delta"){
                mode |= DELTA;
            }else if(input == "mcv256"){
                mode |= MCV256;
            }else if(input.substr(0,6) == "GRMLCM"){
                GRMLCM = atoi(input.substr(6,string::npos).c_str());
            }else
                cerr << "Error: unknown option: --" << input << endl;
        }else{
            string input(&argv[argIndex][1]);
            for(uint32_t i = 0; i < input.size(); i++){
                if(input[i] == 'o'){
                    mode |= COO;
                }else if(input[i] == 'w'){
                    mode |= CSR;
                    mode &= ~COO;
                }else if(input[i] == 'x'){
                    mode &= ~COMPRESS;
                }else if(input[i] == 'c'){
                    mode |= COMPRESS;
                }else if(input[i] == 'e'){
                    mode |= DELTA;
                }else{
                    cerr << "Error: unknown option: -" << input[i] << endl;
                }
            }
        }
        
        argIndex++;
        if((argv[argIndex] == NULL)||(argIndex > 9)||(argIndex > argc - 2)){
            usage(argv[0]);
            return 0;
        }
    }
    cerr << "mode:" << hex << (int)mode << endl << dec; //TODO: fix
    string matrixName(argv[argIndex++]);
    string archiveName(argv[argIndex++]);
    
    int M, N, nnz, *I, *J;
    uint64_t lM, lN, lNnz, *lI, *lJ;
    double *val;
    if(mode & COMPRESS){
        mkdir(archiveName.c_str(), S_IRWXU);
        ofstream headFile((archiveName+"/head").c_str());
        mm_read_unsymmetric_sparse(matrixName.c_str(), &M, &N, &nnz, &val, &I, &J);
        lM = M;
        lN = N;
        lNnz = nnz;
        lI = (uint64_t*)malloc(lNnz * sizeof(uint64_t));
        lJ = (uint64_t*)malloc(lNnz * sizeof(uint64_t));
        for(uint64_t i = 0; i < lNnz; i++){
            lI[i] = I[i];
            lJ[i] = J[i];
        }
        headFile << "\%\%MatrixMarket matrix coordinate real general" << endl;
        headFile << M << " " << N << " " << nnz << endl;
        headFile.close();
    }
    if((mode & DELTA) && (GRMLCM == 0)){
        //TODO: microsoft paper
        ofstream deltaBitsFile((archiveName+"/deltaBits").c_str());
        ofstream deltaLengthsFile((archiveName+"/deltaLengths").c_str());
        uint8_t *deltaLengths = (uint8_t*)malloc(nnz * sizeof(uint8_t));
        uint64_t *deltaValues = (uint64_t*)malloc(nnz * sizeof(uint64_t));
        uint64_t bitCount = 0;
        if(false){
            //TODO: GRMLCM
            //TODO: Coiling
        }else{
            uint64_t count = 0;
            uint64_t deltaVal;
            while(count < lNnz){
                deltaVal = lJ[count];
                deltaLengths[count] = bitLength(lJ[count]);
                deltaValues[count] = deltaVal;
                uint64_t currRow = lI[count];
                count++;
                while(currRow == lI[count]){
                    deltaVal = lJ[count] - lJ[count - 1] - 1;
                    deltaLengths[count] = bitLength(lJ[count] - lJ[count - 1] - 1);
                    deltaValues[count] = deltaVal;
                    count++;
                    if(count >= lNnz)
                        break;
                }
            }
            //TODO: row major bit counts
        }
        //TODO: the real thing
        for(uint64_t i = 0; i < lNnz; i++)
            bitCount += deltaLengths[i];
        deltaBitsFile << bitCount << endl;
        deltaBitsFile.close();
        deltaLengthsFile.write((char*)deltaLengths, lNnz);
        deltaLengthsFile.close();
        cerr << "bitCount:" << bitCount << endl;
        //TODO: count bits
        //TODO: bitpacking
        free(deltaLengths);
        free(deltaValues);
    }
    if((mode & DELTA) && (GRMLCM > 1)){
        sprintf(buffer, "%ld", GRMLCM);
        ofstream deltaBitsFile((archiveName+"/deltaBits"+buffer).c_str());
        ofstream deltaLengthsFile((archiveName+"/deltaLengths"+buffer).c_str());
        uint8_t *deltaLengths = (uint8_t*)malloc(nnz * sizeof(uint8_t));
        uint64_t *deltaValues = (uint64_t*)malloc(nnz * sizeof(uint64_t));
        uint64_t bitCount = 0;
        if(false){
            //TODO: GRMLCM
            //TODO: Coiling
        }else{
            uint64_t count = 0;
            uint64_t deltaVal;
            while(count < lNnz){
                deltaVal = lJ[count];
                deltaLengths[count] = bitLength(lJ[count]);
                deltaValues[count] = deltaVal;
                uint64_t currRow = lI[count] / GRMLCM;
                count++;
                while(currRow == (lI[count] / GRMLCM)){
                    deltaVal = lI[count] - lI[count - 1] - 1 + GRMLCM * (lJ[count] - lJ[count - 1]);
                    deltaLengths[count] = bitLength(deltaVal);
                    deltaValues[count] = deltaVal;
                    count++;
                    if(count >= lNnz)
                        break;
                }
            }
            //TODO: row major bit counts
        }
        //TODO: the real thing
        for(uint64_t i = 0; i < lNnz; i++)
            bitCount += deltaLengths[i];
        deltaBitsFile << bitCount << endl;
        deltaBitsFile.close();
        deltaLengthsFile.write((char*)deltaLengths, lNnz);
        deltaLengthsFile.close();
        cerr << "bitCount:" << bitCount << endl;
        //TODO: count bits
        //TODO: bitpacking
        free(deltaLengths);
        free(deltaValues);
    }
    if((mode & MCV256) && (mode & COMPRESS)){
        cerr << "MCV256 option running" << endl;
        ofstream commonFile((archiveName+"/common").c_str(), ios_base::binary);
        ofstream uncommonFile((archiveName+"/uncommon").c_str(), ios_base::binary);
        

        int* commonValueIndices;
        uint64_t* mcvs;
        mcv((uint64_t*)val, &commonValueIndices, &mcvs, 256, lNnz);
        int mcvValSize = 0;
        for(uint64_t i = 0; i < lNnz; i++){
            if(commonValueIndices[i] == -1){
                mcvValSize += 8;
                commonFile.write((char*)&val[i],8);
            }else{
                uncommonFile.write((char*)&commonValueIndices[i],1);
                mcvValSize += 1;}
        }
        free(mcvs);
        free(commonValueIndices);
        commonFile.close();
        uncommonFile.close();
        cerr << "mcv commpressed val size:" << mcvValSize << endl;
    }
    if((mode & COMPRESS) && (mode &COO)){
        ofstream colFile((archiveName+"/col").c_str(), ios_base::binary);
        colFile.write((char*)I, 4*nnz);
        colFile.close();

        ofstream rowFile((archiveName+"/row").c_str());
        rowFile.write((char*)J, 4*nnz);
        rowFile.close();
        
        ofstream valFile((archiveName+"/val").c_str());
        valFile.write((char*)val, 8*nnz);
        valFile.close();
        
    }else if(mode == (COMPRESS|CSR)){
        ofstream colFile((archiveName+"/col").c_str(), ios_base::binary);
        colFile.write((char*)J, 4*nnz);
        colFile.close();

        ofstream cRowFile((archiveName+"/compressedRow").c_str());
        
        uint64_t count = 0;
        uint64_t prevRow = 0;
        uint64_t index = 1;
        while(index < lNnz){
            if(prevRow != lI[index]){
                count++;
                cRowFile.write((char*)&index, 4);
            }
            prevRow=lI[index];
            index++;
        }
        cRowFile.close();
        
        ofstream valFile((archiveName+"/val").c_str());
        valFile.write((char*)val, 8*nnz);
        valFile.close();
        
    }else if(mode == CSR){
        cerr << "csr extract" << endl;
        ofstream matrixFile(matrixName.c_str());
        ifstream headInFile((archiveName+"/head").c_str());
        string tmp;
        getline(headInFile, tmp);
        matrixFile << tmp << endl;
        headInFile >> M;
        headInFile >> N;
        headInFile >> nnz;
        matrixFile << M << " " << N << " " << nnz << endl;
        
        ifstream colInFile((archiveName+"/col").c_str(), ios_base::binary);
        colInFile.seekg(0, colInFile.end);
        nnz = colInFile.tellg()/4;
        colInFile.seekg(0, colInFile.beg);
        ifstream rowInFile((archiveName+"/compressedRow").c_str(), ios_base::binary);
        ifstream valInFile((archiveName+"/val").c_str(), ios_base::binary);
        uint32_t tmpRow;
        rowInFile.read((char*)&tmpRow, 4);
        uint64_t currRow = 0;
        for(uint32_t i = 0; i < lNnz; i++){
            int tmpCol;
            double tmpVal;
            colInFile.read((char*)&tmpCol, 4);
            valInFile.read((char*)&tmpVal, 8);
            if(tmpRow == i){
                rowInFile.read((char*)&tmpRow, 4);
                currRow++;
            }
            matrixFile << (tmpCol + 1) << " " << (currRow + 1) << " " << tmpVal << endl;
        }
        matrixFile << endl;
        colInFile.close();
        rowInFile.close();
        valInFile.close();
        matrixFile.close();


    }else if(mode == COO){
        ofstream matrixFile(matrixName.c_str());
        ifstream headInFile((archiveName+"/head").c_str());
        string tmp;
        getline(headInFile, tmp);
        matrixFile << tmp << endl;
        headInFile >> M;
        headInFile >> N;
        headInFile >> nnz;
        matrixFile << M << " " << N << " " << nnz << endl;
        
        ifstream colInFile((archiveName+"/col").c_str(), ios_base::binary);
        colInFile.seekg(0, colInFile.end);
        nnz = colInFile.tellg()/4;
        colInFile.seekg(0, colInFile.beg);
        ifstream rowInFile((archiveName+"/row").c_str(), ios_base::binary);
        ifstream valInFile((archiveName+"/val").c_str(), ios_base::binary);
        for(int i = 0; i < nnz; i++){
            int tmpCol, tmpRow;
            double tmpVal;
            colInFile.read((char*)&tmpCol, 4);
            rowInFile.read((char*)&tmpRow, 4);
            valInFile.read((char*)&tmpVal, 8);
            matrixFile << (tmpCol + 1) << " " << (tmpRow + 1) << " " << tmpVal << endl;
        }
        matrixFile << endl;
        colInFile.close();
        rowInFile.close();
        valInFile.close();
        matrixFile.close();

    
    }else{
        cout << "mode not implemented... maybe you should should request it. hahahaha" << endl;
    }
    return 0;
}

void usage(char* programName){
    printf("Usage: %s options source destDir", programName);
}

uint8_t bitLength(uint64_t value){
    uint8_t ret = 0;
    while(value != 0){
        ret++;
        value >>= 1;
    }
    return ret;
}
