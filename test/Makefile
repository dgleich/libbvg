
CFLAGS += -I../include -I../src

allcfiles := $(wildcard *.c)
allprogs := $(allcfiles:.c=)

all: $(allprogs)
clean: 
	rm -rf $(allprogs)

.PHONY: all clean

refill_test: refill_test.o
check_bvgraph: check_bvgraph.o


