# Makefile for amalloc test programs
#CC=tcc
LDLIBS=-lm
LDFLAGS=
CFLAGS=-Wall -O3 -DNDEBUG
DBGLDFLAGS=-g
DBGCFLAGS=-O0 -g -Wall

all: testdarray testdarray_dbg testdarray2 testdarray2_dbg

testdarray: testdarray.o amalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray2: testdarray2.o amalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

amalloc.o: amalloc.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $<

testdarray.o: testdarray.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

testdarray2.o: testdarray2.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

testdarray_dbg: testdarray_dbg.o amalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

testdarray2_dbg: testdarray2_dbg.o amalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

amalloc_dbg.o: amalloc.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

testdarray_dbg.o: testdarray.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

testdarray2_dbg.o: testdarray2.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

clean:
	\rm -f testdarray2.o testdarray2_dbg.o testdarray.o testdarray_dbg.o amalloc.o amalloc_dbg.o
