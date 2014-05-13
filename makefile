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
	g++ -O3 -c spMatrixHelp.cpp

clear :
	rm -rf *.o *.a
