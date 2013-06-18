CFLAGS=-O3
all: testdarray
testdarray: testdarray.o darray.o
	$(CC) -o $@ $^ -lm
darray.o: darray.c
testdarray.o: testdarray.c
clean:
	\rm -f testdarray.o darray.o
