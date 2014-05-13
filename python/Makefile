all: ext test
	
ext: clibbvg.pxd bvg.pyx ../libbvg.a
	python setup.py build_ext --inplace

clean:
	python setup.py clean 
	
../data/harvard500.smat:../data/harvard500.graph ../bvgraph2smat
	../bvgraph2smat ../data/harvard500 -f smat

test: ext ../data/harvard500.smat
	python testunittest.py
	
.PHONY: all clean test ext
