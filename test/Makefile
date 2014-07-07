
LINKER = $(CC)
CFLAGS += -I../include -I../src
LOADLIBES += -lbvg
LDFLAGS += -L../

allcfiles := $(wildcard *.c)
allprogs := $(allcfiles:.c=)

all: $(allprogs) test
clean: 
	rm -rf $(allprogs) $(allcfiles:.c=.o)

.PHONY: all clean test

refill_test: refill_test.o
check_bvgraph: check_bvgraph.o

test: 
	./bitfile_test
	./bitfile_64bit_test
	./refill_test
	./bvgraph_test ../data/harvard500
	./check_bvgraph ../data/harvard500 random 10000
	./check_bvgraph ../data/wb-cs.stanford random 10000
	bzip2 -d ../data/bvheadTail1000.graph.bz2
	echo "unzip dataset for 64 bit test done"
	./bvgraph_64bit_test ../data/bvheadTail1000
	
	


