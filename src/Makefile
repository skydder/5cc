#CFLAGS=-std=c11 -g -static
SRCS= util.c structs.c token.c parser.c codegen.c 5cc.c debag.c
OBJS=$(SRCS:.c=.o)
CC = gcc

5cc: $(OBJS) asm.o vector.o
	$(CC) -o 5cc $(OBJS) asm.o vector.o $(LDFLAGS)

$(OBJS): 5cc.h

asm.o:asm.h
vector.o:vector.h

test: 5cc
	./test.sh

clean:
	rm -f 5cc *.o *~ tmp*

.PHONY: test clean