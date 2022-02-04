CFLAGS=-std=c11 -g -static
SRCS= 5cc.c error.c token.c parser.c 
OBJS=$(SRCS:.c=.o)

5cc: $(OBJS)
	$(CC) -o 5cc $(OBJS) $(LDFLAGS)

$(OBJS): 5cc.h

test: 5cc
	./test.sh

clean:
	rm -f 5cc *.o *~ tmp*

.PHONY: test clean