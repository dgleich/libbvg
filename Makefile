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


LIBBVG_INCLUDE := -Iinclude -Isrc
LIBBVG_SRC := bitfile.c bvgraph.c bvgraph_iterator.c properties.c util.c
LIBBVG_FULL_SRC := $(addprefix $(LIBBVG_SRC_DIR)/,$(LIBBVG_SRC))
BVPAGERANK_INCLUDE := -Iinclude

BVPAGERANK_SRC_DIR := tools/bvpagerank

CFLAGS := $(CFLAGS) -Wall -O2 
CXXFLAGS := $(CXXFLAGS) -Wall -O2 -Iinclude

LOADLIBES += -L. -lbvg

all: everything

# declare phony targets
.PHONY: all lib clean everything

lib: $(LIBBVG_FULL_SRC)
	gcc -c $(LIBBVG_INCLUDE) $(CFLAGS) $(LIBBVG_FULL_SRC)
	ar -rc $(LIBBVGNAME) $(LIBBVG_SRC:.c=.o)

clean:
	$(RM) $(LIBBVG_SRC:.c=.o) $(LIBBVGNAME) $(ALLPROGS) $(ALLOBJS)

$(BVPAGERANKNAME): lib $(BVPAGERANK_SRC_DIR)/bvpagerank.o
	$(CXX) $(LDFLAGS) $(BVPAGERANK_SRC_DIR)/bvpagerank.o -o $(BVPAGERANKNAME) $(LOADLIBES) $(LDLIBS)
	
ALLOBJS += $(BVPAGERANK_SRC_DIR)/bvpagerank.o
ALLPROGS += $(BVPAGERANKNAME)
	
bvgraph2smat : lib tools/bvgraph2smat/bvgraph2smat.o
	$(CXX) $(LDFLAGS) -o bvgraph2smat tools/bvgraph2smat/bvgraph2smat.o $(LOADLIBES) $(LDLIBS)

ALLOBJS += tools/bvgraph2smat/bvgraph2smat.o
ALLPROGS += bvgraph2smat

everything: lib $(BVPAGERANKNAME) bvgraph2smat

