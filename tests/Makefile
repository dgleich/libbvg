#
# Testing makefile
# Wei-Yen Day
# 2011-09-06

SRC := test.c
LIB := -L ../

.PHONY: all clean

all:
	gcc $(SRC) -lbvg $(LIB) 

clean:
	$(RM) $(SRC:.c=.o)
