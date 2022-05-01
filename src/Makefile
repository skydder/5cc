#CFLAGS=-std=c11 -g -static
SRCS= 5cc.c util.c token.c parser.c codegen.c 
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