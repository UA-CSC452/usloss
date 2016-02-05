CC=gcc
#CFLAGS=-Wall -I/home/cs452/include -L/home/cs452/lib
CFLAGS=-Wall -I../src
LIBS = -lusloss
LDFLAGS += -L../src

test1: test1.o
	$(CC) $(LDFLAGS) -o test1 test1.o $(LIBS)

clean:
	rm -f *.o test1
