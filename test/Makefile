
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
#	./bitfile_test
#	./refill_test
#	./bvgraph_test ../data/harvard500
#	./check_bvgraph ../data/harvard500 random 10000
#	./check_bvgraph ../data/wb-cs.stanford random 10000
	./bitfile_64bit_test
#	./bvgraph_test /scratch2/dgleich/data/clueweb12/clueweb12
	


