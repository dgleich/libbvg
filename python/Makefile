all: ext test
	
ext: 
	python setup.py build_ext --inplace

clean:
	rm -rf libbvg.so libbvg.c build
	
../data/harvard500.smat:../data/harvard500.graph ../bvgraph2smat
	../bvgraph2smat ../data/harvard500 -f smat

test: ext ../data/harvard500.smat
	python testunittest.py
	
.PHONY: all clean test ext
