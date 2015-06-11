prefix=$(HOME)
all : spm smac spMatrixHelp.o

mcv.o : mcv.cpp
	g++ -O3 -c mcv.cpp

mmio.o : mmio.c
	g++ -O3 -c mmio.c

smac : mmio.o mcv.o smac.cpp
	g++ -O3 -Wall -o smac smac.cpp mmio.o mcv.o

spm : mmio.o spm.cpp
	g++ -O3 -o spm spm.cpp mmio.o

spMatrixHelp.o : spMatrixHelp.hpp spMatrixHelp.cpp
	g++ -std=gnu++0x -O3 -c spMatrixHelp.cpp

release :
	cp spMatrixHelp.o $(prefix)/lib/.
	cp spMatrixHelp.hpp $(prefix)/include/.

rcr : rcr.cpp
	g++ -std=gnu++0x -o rcr rcr.cpp

test: testRcr testSmallQcd

testRcr: rcr
	rcr < example.mtx > example.rcr

testCant:
	rcr < benchmark/cant.mtx > cant.rcr

testDense:
	rcr < benchmark/dense2.mtx > dense.rcr

testQcd:
	rcr < benchmark/qcd5_4.mtx > qcd.rcr

testSmallQcd:
	rcr < qcdSmall.mtx > smallQcd.rcr

testSmac : smac
	smac

clear :
	rm -rf *.o *.a

vim :
	vim -p makefile rcr.cpp
