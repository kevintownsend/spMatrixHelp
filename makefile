prefix=$(HOME)
all : spm smac spMatrixHelp.o rcr

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

%.rcr : %.mtx
	rcr < $< > $@

testAll: benchmark/consph.rcr benchmark/cant.rcr benchmark/cop20k_A.rcr benchmark/dense2.rcr benchmark/mac_econ_fwd500.rcr benchmark/mc2depi.rcr benchmark/pdb1HYS.rcr benchmark/pwtk.rcr benchmark/qcd5_4.rcr benchmark/rail4284.rcr benchmark/rma10.rcr benchmark/scircuit.rcr benchmark/shipsec1.rcr benchmark/webbase-1M.rcr

cleanRcr:
	rm benchmark/*.rcr log

runBenchmark: cleanRcr testAll
	sum.py

benchmark/consph.rcr : benchmark/consph.mtx

#%.bin : %.fasta.gz
#	gunzip -c $< | ./fasta2bin > $@

testSmallQcd:
	rcr < qcdSmall.mtx > smallQcd.rcr

testSmac : smac
	smac

clean :
	rm -rf *.o *.a

vim :
	vim -p makefile rcr.cpp ../shepard/reference/Makefile
