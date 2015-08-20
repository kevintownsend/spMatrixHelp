#!/usr/bin/env python3
from glob import *
from subprocess import *
import os
import sys
from math import *

optimalWidth = 8
optimalHeight = 128
matrixPaths = glob("./benchmark/*.mtx")
matrices = []
for m in matrixPaths:
    matrices.append(m.replace("./benchmark/","").replace(".mtx", ""))
dataFile = open("data.csv", "w")
dataFile.write("matrices")

matrixFile = open("./benchmark/webbase-1M.mtx", "r")
matrixFile.readline()
line = matrixFile.readline()
size = int(line.split(" ")[2])
matrixFile.close()
results = []
for j in range(10):
    call(["rcr", "-c", "--subwidth=8", "--subheight=128", "--huffmandepth=" + repr(2**j), "./benchmark/webbase-1M.mtx", "./tmp/webbase-1M.rcr"])
    filesize = os.stat("./tmp/webbase-1M.rcr").st_size
    results.append(filesize / size)

print(results)
