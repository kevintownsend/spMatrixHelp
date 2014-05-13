/**
 * Copyright 2014 Kevin Townsend
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdlib>
#include <iostream>
#include <stdint.h>
#include "mcv.h"
#define uint64 uint64_t

using namespace std;
int find256Mcv(uint64** values, uint64* source, int nnz);

void mcv(uint64_t* values, int** commonValueIndices, uint64_t** mcvs, int mcvSize, uint64_t nnz){
    //TODO: finish
    //TODO: assert mcvSize=256
    if(mcvSize != 256){
        cerr << "only 256 common values supported currently" << endl;
    }else{
        uint64_t* depricatedFormat = (uint64_t*)malloc(2 * nnz * sizeof(uint64_t));
        for(uint64_t i = 0; i < nnz; i++){
            depricatedFormat[2 * i + 1] = values[i];
        }
        cerr << "entering find256mcvs" << endl;
        int ret = find256Mcv(mcvs, depricatedFormat, nnz);
        cerr << "exiting find256mcvs" << endl;
        free(depricatedFormat);
        *commonValueIndices = (int*)malloc(nnz * sizeof(int));
        for(uint64_t i = 0; i < nnz; i++){
            (*commonValueIndices)[i] = -1;
            for(int j = 0; j < 256; j++){
                if(values[i] == (*mcvs)[j]){
                    (*commonValueIndices)[i] = j;
                    break;
                }
            }
        }
    }
}

typedef struct{
    uint64 value;
    int left;
    int right;
    int parent;
    int count;
    int smaller;
    int larger;
}valueNode;

void clearValueNode(valueNode* node){
    node->value = 0;
    node->left = -1;
    node->right = -1;
    node->parent = -1;
    node->count = 0;
    node->smaller = -1;
    node->larger = -1;
    return;
}

void dfsMaxCount(valueNode* tree, int currNode, int* counts, uint64* values){
    if(tree[currNode].left != -1){
        dfsMaxCount(tree, tree[currNode].left, counts, values);
    }
    if(tree[currNode].right != -1){
        dfsMaxCount(tree, tree[currNode].right, counts, values);
    }
    int tmpCount = tree[currNode].count;
    uint64 tmpVal = tree[currNode].value;
    int swapCount;
    uint64 swapVal;
    for(int i = 0; i < 256; i++){
        if(counts[i] < tmpCount){
            swapCount = counts[i];
            counts[i] = tmpCount;
            tmpCount = swapCount;
            swapVal = values[i];
            values[i] = tmpVal;
            tmpVal = swapVal;
        }
    }
}

int find256Mcv(uint64** values, uint64* source, int nnz){
    *values = (uint64*)malloc(256 * sizeof(uint64));
    valueNode* tree = (valueNode*)malloc(nnz * sizeof(valueNode));
    clearValueNode(tree);
    int root = 0;
    int currNode = 0;
    int nodeCount = 1;
    tree[0].value = source[1];
    tree[0].count = 1;
    for(int i = 1; i < nnz; i++){
        currNode = root;
        while(true){
            if(tree[currNode].value == source[i * 2 + 1]){
                tree[currNode].count += 1;
                break;
            }else if(source[i * 2 + 1] < tree[currNode].value){
                if(tree[currNode].left == -1){
                    clearValueNode(&tree[nodeCount]);
                    tree[nodeCount].value = source[i * 2 + 1];
                    tree[nodeCount].parent = currNode;
                    tree[nodeCount].count = 1;
                    tree[currNode].left = nodeCount;
                    nodeCount++;
                    break;
                }else{
                    currNode = tree[currNode].left;
                }
            }else if(source[i * 2 + 1] > tree[currNode].value){
                if(tree[currNode].right == -1){
                    clearValueNode(&tree[nodeCount]);
                    tree[nodeCount].value = source[i * 2 + 1];
                    tree[nodeCount].parent = currNode;
                    tree[nodeCount].count = 1;
                    tree[currNode].right = nodeCount;
                    nodeCount++;
                    break;
                }else{
                    currNode = tree[currNode].right;
                }
            }
        }
    }
    int counts[256];
    for(int i = 0; i < 256; i++){
        (*values)[i] = 0;
        counts[i] = 0;
    }
    dfsMaxCount(tree, root, counts, *values);
    free(tree);
    return nodeCount;
}
