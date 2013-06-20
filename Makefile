# Makefile for amalloc test programs
#CC=tcc
LDLIBS=-lm
LDFLAGS=
CFLAGS=-Wall -O3 -DNDEBUG
DBGLDFLAGS=-g
DBGCFLAGS= -Wall -O0 -DDEBUG -g

all: testdarray testdarray_dbg testdarray2 testdarray2_dbg testdarray3 testdarray3_dbg amalloc2dspeed amalloc2dspeed_dbg

testdarray: testdarray.o amalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray2: testdarray2.o amalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray3: testdarray3.o amalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

amalloc2dspeed: amalloc2dspeed.o amalloc.o pass.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

amalloc.o: amalloc.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $<

testdarray.o: testdarray.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

testdarray2.o: testdarray2.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

testdarray3.o: testdarray3.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

pass.o: pass.c
	${CC} -O0 -g -c -o $@ $<

amalloc2dspeed.o: amalloc2dspeed.c amalloc.h cstopwatch.h
	${CC} ${CFLAGS} -c -o $@ $< 

testdarray_dbg: testdarray_dbg.o amalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

testdarray2_dbg: testdarray2_dbg.o amalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

testdarray3_dbg: testdarray3_dbg.o amalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

amalloc2dspeed_dbg: amalloc2dspeed_dbg.o amalloc.o pass.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

amalloc_dbg.o: amalloc.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

testdarray_dbg.o: testdarray.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

testdarray2_dbg.o: testdarray2.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

testdarray3_dbg.o: testdarray3.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

amalloc2dspeed_dbg.o: amalloc2dspeed.c amalloc.h  cstopwatch.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

clean:
	\rm -f testdarray3.o testdarray3_dbg.o testdarray2.o testdarray2_dbg.o testdarray.o testdarray_dbg.o amalloc.o amalloc_dbg.o amalloc2dspeed.o amalloc2dspeed_dbg.o pass.o
