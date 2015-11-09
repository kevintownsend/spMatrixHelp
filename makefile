prefix=$(HOME)
all : spm

vim :
	vim -p makefile spm.hpp spm.cpp rcrToHard.cpp

rcrToHard : rcrToHard.cpp rcrHelper.hpp rcr.hpp
	g++ -std=c++11 -O3 -o rcrToHard rcrToHard.cpp rcrHelper.o

rcrHelper.o : rcrHelper.cpp rcrHelper.hpp
	g++ -std=c++11 -O3 -c rcrHelper.cpp

mkHex : mkHex.cpp
	g++ -O3 -o mkHex mkHex.cpp

benchmark:
	mkdir benchmark
	wget http://www.nvidia.com/content/NV_Research/matrices.zip
	unzip matrices.zip -d benchmark/.
	cp example.mtx benchmark/.

run : spm
	spm -c example.mtx example.spm
	spm -d example.spm exampleAfter.mtx
	cat exampleAfter.mtx

run2 : benchmark patternize rcr
	rcrScript.py

patternize: patternize.cpp
	g++ -O3 -std=c++11 -o patternize patternize.cpp

mcv.o : mcv.cpp
	g++ -O3 -c mcv.cpp

mmio.o : mmio.c
	g++ -O3 -c mmio.c

smac : mmio.o mcv.o smac.cpp
	g++ -O3 -Wall -o smac smac.cpp mmio.o mcv.o

spm : spm.cpp spm.hpp
	g++ -std=gnu++0x -O0 -o spm spm.cpp

spMatrixHelp.o : spMatrixHelp.hpp spMatrixHelp.cpp
	g++ -std=gnu++0x -O3 -c spMatrixHelp.cpp

release :
	cp spMatrixHelp.o $(prefix)/lib/.
	cp spMatrixHelp.hpp $(prefix)/include/.

rcr : rcr.cpp rcrHelper.o rcr.hpp
	g++ -O3 -std=c++11 -o rcr rcr.cpp rcrHelper.o

test: testRcr testSmallQcd

testRcr: rcr
	time rcr -c --subheight=4 example.mtx example.rcr
	echo "part 2:"
	time rcr -x --subheight=4 example.rcr exampleProcessed.mtx
	echo "check:"
	diff example.mtx exampleProcessed.mtx

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
	rcr qcdSmall.mtx smallQcd.rcr

testSmac : smac
	smac

clean :
	rm -rf *.o *.a
