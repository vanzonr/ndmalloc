# Makefile for amalloc test programs
CC=gcc
LDLIBS=-lm
#LDFLAGS=-g -O3 -fast
LDFLAGS=-g -O3 
#-march=native  
CFLAGS=-Wall -g -O3 -march=native -DNDEBUG -std=c99
#-fast 
#-fast
DBGLDFLAGS=-g -gdwarf-2
DBGCFLAGS= -Wall -O0 -DDEBUG -g
#PROFLAG=-pg
PRFLDFLAGS=-g ${PROFLAG}
PRFCFLAGS= -Wall -O2 -DDEBUG -g ${PROFLAG}

all: testdarray testdarray_dbg testdarray2 testdarray2_dbg testdarray3 testdarray3_dbg amalloc2dspeed amalloc2dspeed_dbg testdarray5 testdarray5_dbg

testdarray: testdarray.o amalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray2: testdarray2.o amalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray3: testdarray3.o amalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray5: testdarray5.o amalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

amalloc2dspeed: amalloc2dspeed.o \
                amalloc2dspeed-auto.o \
                amalloc2dspeed-exact.o \
                amalloc2dspeed-dynamic.o \
                amalloc2dspeed-amalloc.o \
                amalloc.o pass.o test_damalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

amalloc.o: amalloc.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $<

test_damalloc.o: test_damalloc.c test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $<

testdarray.o: testdarray.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

testdarray2.o: testdarray2.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

testdarray3.o: testdarray3.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

testdarray5.o: testdarray5.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

pass.o: pass.c
	${CC} -O0 -g -c -o $@ $<

pass_prf.o: pass.c
	${CC} -O0 -g ${PROFLAG} -c -o $@ $<

amalloc2dspeed.o: amalloc2dspeed.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

amalloc2dspeed-dynamic.o: amalloc2dspeed-dynamic.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

amalloc2dspeed-auto.o: amalloc2dspeed-auto.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

amalloc2dspeed-amalloc.o: amalloc2dspeed-amalloc.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

amalloc2dspeed-exact.o: amalloc2dspeed-exact.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

testdarray_dbg: testdarray_dbg.o amalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

testdarray2_dbg: testdarray2_dbg.o amalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

testdarray3_dbg: testdarray3_dbg.o amalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

testdarray5_dbg: testdarray5_dbg.o amalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

amalloc2dspeed_dbg: amalloc2dspeed_dbg.o \
                amalloc2dspeed-auto_dbg.o \
                amalloc2dspeed-exact_dbg.o \
                amalloc2dspeed-dynamic_dbg.o \
                amalloc2dspeed-amalloc_dbg.o \
                amalloc_dbg.o pass.o test_damalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

amalloc_dbg.o: amalloc.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

test_damalloc_dbg.o: test_damalloc.c test_damalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

testdarray_dbg.o: testdarray.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

testdarray2_dbg.o: testdarray2.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

testdarray3_dbg.o: testdarray3.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

testdarray5_dbg.o: testdarray5.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

amalloc2dspeed_dbg.o: amalloc2dspeed.c amalloc.h  cstopwatch.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

amalloc_prf.o: amalloc.c amalloc.h
	${CC} ${PRFCFLAGS} -c -o $@ $<

test_damalloc_prf.o: test_damalloc.c test_damalloc.h
	${CC} ${PRFCFLAGS} -c -o $@ $<

amalloc2dspeed_prf.o: amalloc2dspeed.c amalloc.h  cstopwatch.h
	${CC} ${PRFCFLAGS} -c -o $@ $< 

amalloc2dspeed_prf: amalloc2dspeed_prf.o \
                    amalloc2dspeed-auto_prf.o \
                    amalloc2dspeed-exact_prf.o \
                    amalloc2dspeed-dynamic_prf.o \
                    amalloc2dspeed-amalloc_prf.o \
                    amalloc_prf.o pass_prf.o test_damalloc_prf.o
	${CC} ${PRFLDFLAGS} -o $@ $^ ${LDLIBS}

clean:
	\rm -f amalloc2dspeed-amalloc.o amalloc_dbg.o testdarray3_dbg.o amalloc2dspeed-auto.o amalloc.o testdarray3.o amalloc2dspeed_dbg.o pass.o testdarray_dbg.o amalloc2dspeed-dynamic.o test_damalloc.o testdarray.o amalloc2dspeed-exact.o testdarray2_dbg.o amalloc2dspeed.o testdarray2.o
