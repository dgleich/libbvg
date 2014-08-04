all: pylibbvg

clean:
	python setup.py clean
	$(RM) -r data include src test 

LIBBVG_SRC_DIR := ../../src
LIBBVG_SRC := bitfile.c bvgraph.c bvgraph_iterator.c bvgraph_random.c bvgraphfun.c properties.c util.c bvgraph_internal.h bvgraph_inline_io.h
LIBBVG_FULL_SRC := $(addprefix $(LIBBVG_SRC_DIR)/,$(LIBBVG_SRC))

LIBBVG_INCLUDE_DIR := ../../include
LIBBVG_INCLUDE := bitfile.h bvgraph.h gamma.h uthash.h zeta.h pstdint.h
LIBBVG_FULL_INCLUDE := $(addprefix $(LIBBVG_INCLUDE_DIR)/,$(LIBBVG_INCLUDE))

LIBBVG_DATA_DIR := ../../data
LIBBVG_DATA := harvard500.graph harvard500.offsets harvard500.properties wb-cs.stanford.graph wb-cs.stanford.properties
LIBBVG_FULL_DATA := $(addprefix $(LIBBVG_DATA_DIR)/,$(LIBBVG_DATA))
LIBBVG_FULL_DATA += ../../test/bv_head_tail_1000.graph.bz2 ../../test/bv_head_tail_1000.properties

TEST_DIR := ../test
TEST_SRC := test_networkx.py testinterface.py testlibbvg.py testunittest.py test_64bit_bvg.py
TEST_FULL_SRC := $(addprefix $(TEST_DIR)/,$(TEST_SRC))
	
DEPS := README.rst MANIFEST.in LICENSE.txt ../bvg.pyx ../clibbvg.pxd
DEPS += $(LIBBVG_FULL_SRC)
DEPS += $(LIBBVG_FULL_INCLUDE)
DEPS += $(LIBBVG_FULL_DATA)
DEPS += $(TEST_FULL_SRC)
	
pylibbvg: $(DEPS)
	mkdir src && mkdir include && mkdir data && mkdir test
	cp ../bvg.pyx src
	cp ../clibbvg.pxd src
	cp $(LIBBVG_FULL_SRC) src
	cp $(LIBBVG_FULL_INCLUDE) include
	cp $(TEST_FULL_SRC) test
	cp $(LIBBVG_FULL_DATA) data
	

.PHONY: all clean pylibbvg
