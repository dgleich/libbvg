
LINKER = $(CC)
CFLAGS += -I../include -I../src
LOADLIBES += -lbvg
LDFLAGS += -L../

allcfiles := $(wildcard *.c)
allprogs := $(allcfiles:.c=)

all: $(allprogs)
clean: 
	rm -rf $(allprogs) $(allcfiles:.c=.o)

.PHONY: all clean

refill_test: refill_test.o
check_bvgraph: check_bvgraph.o


