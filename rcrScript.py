#!/usr/bin/env python3
from glob import *
from subprocess import *
import os
import sys
from math import *

ResultsFile = open("Results", "w")
optimalWidth = -1
optimalHeight = -1
matrixPaths = glob("./benchmark/*.mtx")
matrices = []
for m in matrixPaths:
    matrices.append(m.replace("./benchmark/","").replace(".mtx", ""))
dataFile = open("data.csv", "w")
dataFile.write("matrices")
sizes = []
for m in matrices:
    matrixFile = open("./benchmark/" + m + ".mtx", "r")
    matrixFile.readline()
    line = matrixFile.readline()
    sizes.append(int(line.split(" ")[2]))
    matrixFile.close()
#matrices = matrices[:2]
if(optimalWidth == -1):
    compressionRatios = []
    for m in matrices:
        compressionRatios.append([])
        dataFile.write(", " + m)
        matrixFile = open("./benchmark/" + m + ".mtx", "r")
        matrixFile.readline()
        line = matrixFile.readline()
        sizes.append(int(line.split(" ")[2]))
        matrixFile.close()
        i = 1
        while(i <= 512):
            compressionRatios[-1].append([])
            j = 1
            while(j <= 512):
                call(["spm", "-c", "--subwidth=" + repr(i), "--subheight=" + repr(j), "--huffmandepth=32", "--maxhuffmanlength=9", "./benchmark/" + m + ".mtx", "./tmp/" + m + ".spm"])
                filesize = os.stat("./tmp/" + m + ".spm").st_size
                compressionRatios[-1][-1].append(filesize / sizes[-1]);
                print("filesize: " + repr(filesize))
                j *= 2
            i *= 2
    averages = []
    for i in range(len(compressionRatios[0])):
        averages.append([])
        for j in range(len(compressionRatios[0][0])):
            total = 0.0
            for k in range(len(compressionRatios)):
                total += compressionRatios[k][i][j];
            averages[-1].append(total / len(compressionRatios))

    dataFile.close()

    minV = 100
    minI = 0
    minJ = 0
    for i in range(len(averages)):
        for j in range(len(averages[0])):
            if(minV > averages[i][j]):
                minV = averages[i][j]
                minI = 2**i
                minJ = 2**j

    print("subWidth: " + repr(minI) + " subHeight: " + repr(minJ) + " ratio: " + repr(minV))
    ResultsFile.write("subWidth: " + repr(minI) + " subHeight: " + repr(minJ) + " ratio: " + repr(minV) + "\n")
    ResultsFile.write(repr(averages) + "\n")

results = []
for i in range(len(matrices)):
    results.append([])
    for j in range(10):
        call(["spm", "-c", "--subwidth=8", "--subheight=128", "--huffmandepth=" + repr(2**j), "./benchmark/" + matrices[i] + ".mtx", "./tmp/" + matrices[i] + ".spm"])
        filesize = os.stat("./tmp/" + matrices[i] + ".spm").st_size
        results[-1].append(filesize / sizes[i])

averages = []
for i in range(10):
    total = 0
    for j in range(len(matrices)):
        total += results[j][i]
    averages.append(total / len(matrices))
print("results: " + repr(results))
print("averages: " + repr(averages))
ResultsFile.write("Huffman Analysis\n")
ResultsFile.write("results: " + repr(results) + "\n")
ResultsFile.write("averages: " + repr(averages) + "\n")
print("optimal: " + repr(optimalWidth) + " " + repr(optimalHeight))
print("bye crule world")
sys.exit(0)

