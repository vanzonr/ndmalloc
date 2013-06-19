# Makefile for darray test programs
#CC=tcc
LDLIBS=-lm
LDFLAGS=
CFLAGS=-O3 -DNDEBUG
DBGLDFLAGS=-g
DBGCFLAGS=-O0 -g

all: testdarray testdarray_dbg testdarray2 testdarray2_dbg

testdarray: testdarray.o darray.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray2: testdarray2.o darray.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

darray.o: darray.c
	${CC} ${CFLAGS} -c -o $@ $^ 

testdarray.o: testdarray.c
	${CC} ${CFLAGS} -c -o $@ $^ 

testdarray2.o: testdarray2.c
	${CC} ${CFLAGS} -c -o $@ $^ 

testdarray_dbg: testdarray_dbg.o darray_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

testdarray2_dbg: testdarray2_dbg.o darray_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

darray_dbg.o: darray.c
	${CC} ${DBGCFLAGS} -c -o $@ $^ 

testdarray_dbg.o: testdarray.c
	${CC} ${DBGCFLAGS} -c -o $@ $^ 

testdarray2_dbg.o: testdarray2.c
	${CC} ${DBGCFLAGS} -c -o $@ $^ 

clean:
	\rm -f testdarray.o darray.o testdarray_dbg.o darray_dbg.o
