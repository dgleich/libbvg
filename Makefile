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
LIBBVG_SRC := bitfile.c bvgraph.c bvgraph_iterator.c bvgraph_random.c bvgraphfun.c properties.c util.c eflist.c
LIBBVG_FULL_SRC := $(addprefix $(LIBBVG_SRC_DIR)/,$(LIBBVG_SRC))

BVPAGERANK_INCLUDE := -Iinclude
BVPAGERANK_SRC_DIR := tools/bvpagerank

CFLAGS := $(CFLAGS) -Wall -O2 -Wextra -Wno-unused-parameter -fPIC $(LIBBVG_INCLUDE)
CXXFLAGS := $(CXXFLAGS) -Wall -O2 -Iinclude

LOADLIBES += -L. -lbvg

all: everything

# declare phony targets
.PHONY: all lib clean everything python docs

#lib: $(LIBBVG_FULL_SRC)
	#gcc -c $(LIBBVG_INCLUDE) $(CFLAGS) $(LIBBVG_FULL_SRC)
	#ar -rcs $(LIBBVGNAME) $(LIBBVG_SRC:.c=.o)

lib: $(LIBBVGNAME)

$(LIBBVGNAME): $(LIBBVG_FULL_SRC:.c=.o)
	ar -rcs $(LIBBVGNAME) $(LIBBVG_FULL_SRC:.c=.o)

clean:
	$(RM) $(LIBBVG_FULL_SRC:.c=.o) $(LIBBVGNAME) $(ALLPROGS) $(ALLOBJS)
	cd test && $(MAKE) clean
	cd python && $(MAKE) clean
	$(RM) -r html/docs doxyfile.inc

$(BVPAGERANKNAME): lib $(BVPAGERANK_SRC_DIR)/bvpagerank.o
	$(CXX) $(LDFLAGS) $(BVPAGERANK_SRC_DIR)/bvpagerank.o -o $(BVPAGERANKNAME) $(LOADLIBES) $(LDLIBS)
	
ALLOBJS += $(BVPAGERANK_SRC_DIR)/bvpagerank.o
ALLPROGS += $(BVPAGERANKNAME)
	
bvgraph2smat : lib tools/bvgraph2smat/bvgraph2smat.o
	$(CXX) $(LDFLAGS) -o bvgraph2smat tools/bvgraph2smat/bvgraph2smat.o $(LOADLIBES) $(LDLIBS)

ALLOBJS += tools/bvgraph2smat/bvgraph2smat.o
ALLPROGS += bvgraph2smat

everything: lib $(BVPAGERANKNAME) bvgraph2smat test python

test: lib
	cd test && $(MAKE) small

python: lib
	cd python && $(MAKE) all
	
SRCS := $(OBJS:.o=.c)
SRCDIRS := ./src
SRCDIRS += ./include
SRCDIRS += ./src/mainpage.dox

doxyfile.inc: $(SRCDIRS)
	echo INPUT = $(SRCDIRS) > doxyfile.inc
	echo FILE_PATTERNS = *.h *.c $(SRCS) >> doxyfile.inc
	
docs: doxyfile.inc $(SRCS)
	mkdir -p html
	doxygen doxyfile.mk
