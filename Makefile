#
# Makefile for libbvg
#
# David Gleich
# Copyright, Stanford University, 2008
# 4 February 2008
#

LIBBVGNAME := libbvg.a
LIBBVG_SRC_DIR := src
BVPAGERANKNAME := bvpr
BVPAGERANK_SRC_DIR := tools/bvpagerank

CFLAGS := $(CFLAGS) -Wall -O2 -Wextra -Wno-unused-parameter
CXXFLAGS := $(CXXFLAGS) -Wall -O2

LIBBVG_INCLUDE := -Iinclude -Isrc
LIBBVG_SRC := bitfile.c bvgraph.c bvgraph_iterator.c bvgraph_random.c bvgraphfun.c properties.c util.c
LIBBVG_FULL_SRC := $(addprefix $(LIBBVG_SRC_DIR)/,$(LIBBVG_SRC))
BVPAGERANK_INCLUDE := -Iinclude

# declare phony targets
.PHONY: all lib clean test

all: lib $(BVPAGERANKNAME) test

lib: $(LIBBVG_FULL_SRC)
	gcc -c $(LIBBVG_INCLUDE) $(CFLAGS) $(LIBBVG_FULL_SRC)
	ar -rc $(LIBBVGNAME) $(LIBBVG_SRC:.c=.o)

clean:
	$(RM) $(LIBBVG_SRC:.c=.o) $(LIBBVGNAME)

$(BVPAGERANKNAME): lib $(BVPAGERANK_SRC_DIR)/bvpagerank.cc
	$(CXX) -c $(BVPAGERANK_INCLUDE) $(CXXFLAGS) $(BVPAGERANK_SRC_DIR)/bvpagerank.cc
	$(CXX) bvpagerank.o -o $(BVPAGERANKNAME) -lbvg -L.

test: lib
	cd tests && $(MAKE)

