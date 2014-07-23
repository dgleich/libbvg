all: ext test pylibbvg
	
ext: clibbvg.pxd bvg.pyx ../libbvg.a
	python setup.py build_ext --inplace

clean:
	python setup.py clean 
	$(RM) -r pylibbvg
	
../data/harvard500.smat:../data/harvard500.graph ../bvgraph2smat
	../bvgraph2smat ../data/harvard500 -f smat

test: ext ../data/harvard500.smat
	python testunittest.py
	
DEPS:= README.rst MANIFEST.in LICENSE.txt bvg.pyx clibbvg.pxd setup.in test_networkx.py testinterface.py testlibbvg.py testunittest.py
DEPS+= ../include/bitfile.h ../include/bvgraph.h ../include/gamma.h ../include/uthash.h ../include/zeta.h ../include/pstdint.h
DEPS+= ../src/bitfile.c ../src/bvgraph_inline_io.h ../src/bvgraph_internal.h ../src/bvgraph_iterator.c ../src/bvgraph_random.c
DEPS+= ../src/bvgraph.c ../src/properties.c ../src/util.c
DEPS+= ../data/harvard500.graph ../data/harvard500.offsets ../data/harvard500.properties
DEPS+= ../data/wb-cs.stanford.graph ../data/wb-cs.stanford.properties
DEPS+= ../test/bv_head_tail_1000.graph.bz2 ../test/bv_head_tail_1000.properties
	
pylibbvg: $(DEPS)
	mkdir pylibbvg
	cp README.rst pylibbvg/
	cp LICENSE.txt pylibbvg/
	cp MANIFEST.in pylibbvg/
	cp setup.in pylibbvg/
	cd pylibbvg && mv setup.in setup.py
	cd pylibbvg && mkdir src && mkdir include && mkdir data && mkdir test
	cp bvg.pyx pylibbvg/src
	cp clibbvg.pxd pylibbvg/src/
	cp ../src/bitfile.c pylibbvg/src/
	cp ../src/bvgraph_inline_io.h pylibbvg/src/
	cp ../src/bvgraph_internal.h pylibbvg/src/
	cp ../src/bvgraph_iterator.c pylibbvg/src/
	cp ../src/bvgraph_random.c pylibbvg/src/
	cp ../src/bvgraph.c pylibbvg/src/
	cp ../src/properties.c pylibbvg/src/
	cp ../src/util.c pylibbvg/src/
	cp ../include/bitfile.h pylibbvg/include/
	cp ../include/bvgraph.h pylibbvg/include/
	cp ../include/gamma.h pylibbvg/include/
	cp ../include/uthash.h pylibbvg/include/
	cp ../include/zeta.h pylibbvg/include/
	cp ../include/pstdint.h pylibbvg/include/
	cp test_networkx.py pylibbvg/test/
	cp testinterface.py pylibbvg/test/
	cp testlibbvg.py pylibbvg/test/
	cp testunittest.py pylibbvg/test/
	cp test_64bit_bvg.py pylibbvg/test/
	cp ../data/harvard500.graph pylibbvg/data/
	cp ../data/harvard500.offsets pylibbvg/data/
	cp ../data/harvard500.properties pylibbvg/data/
	cp ../data/wb-cs.stanford.graph pylibbvg/data/
	cp ../data/wb-cs.stanford.properties pylibbvg/data/
	cp ../test/bv_head_tail_1000.graph.bz2 pylibbvg/data/
	cp ../test/bv_head_tail_1000.properties pylibbvg/data/
	

.PHONY: all clean test ext pylibbvg
