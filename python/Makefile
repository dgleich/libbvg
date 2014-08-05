all: ext test
	
ext: clibbvg.pxd bvg.pyx ../libbvg.a
	python setup.py build_ext --inplace

clean:
	python setup.py clean 
	
../data/harvard500.smat:../data/harvard500.graph ../bvgraph2smat
	../bvgraph2smat ../data/harvard500 -f smat

test: ext ../data/harvard500.smat ../test/bv_head_tail_1000.graph
	PYTHONPATH="." python ./test/testunittest.py
	PYTHONPATH="." python ./test/test_head_tail_1000_64bit.py ../test/bv_head_tail_1000
	PYTHONPATH="." python ./test/test_networkx.py
	
../test/bv_head_tail_1000.graph:
	$(error please run make test in the main directory first)
	
	
.PHONY: all clean test ext
