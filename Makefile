# Makefile for amalloc test programs
CC=gcc
LDLIBS=-lm
#LDFLAGS=-g -O3 -fast
LDFLAGS=-g -gdwarf-2 -O3 -pthread
#-march=native  
CFLAGS=-Wall -g -gdwarf-2 -O3 -march=native -DNDEBUG -std=c99 
AMALLOCCFLAGS=-Wall -DAREG_PTHREAD_LOCK -g -gdwarf-2 -O3 -march=native -ansi -pedantic -finline-limit=128 
#-fast 
#-fast
DBGLDFLAGS=-g -gdwarf-2 -pthread
DBGCFLAGS= -Wall -O0 -DDEBUG -g -gdwarf-2
AMALLOCDBGCFLAGS=-Wall -DAREG_PTHREAD_LOCK -g -gdwarf-2 -O0 -ansi -pedantic
#PROFLAG=-pg
PRFLDFLAGS=-g -gdwarf-2 -pthread ${PROFLAG}
PRFCFLAGS= -Wall -O2 -DDEBUG -g  -gdwarf-2 ${PROFLAG}

all: testdarray testdarray_dbg testdarray2 testdarray2_dbg testdarray3 testdarray3_dbg amalloc2dspeed amalloc2dspeed_dbg testdarray5 testdarray5_dbg aregtest

aregtest: aregtest.o areg.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray: testdarray.o amalloc.o areg.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray2: testdarray2.o amalloc.o areg.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray3: testdarray3.o amalloc.o areg.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

testdarray5: testdarray5.o amalloc.o areg.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

amalloc2dspeed: amalloc2dspeed.o \
                amalloc2dspeed-auto.o \
                amalloc2dspeed-exact.o \
                amalloc2dspeed-dynamic.o \
                amalloc2dspeed-amalloc.o \
                amalloc.o areg.o pass.o test_damalloc.o
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

amalloc.o: amalloc.c amalloc.h
	${CC} ${AMALLOCCFLAGS} -c -o $@ $<

areg.o: areg.c areg.h
	${CC} ${AMALLOCCFLAGS} -c -o $@ $<

aregtest.o: aregtest.c areg.h
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

testdarray_dbg: testdarray_dbg.o amalloc_dbg.o areg_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

testdarray2_dbg: testdarray2_dbg.o amalloc_dbg.o areg_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

testdarray3_dbg: testdarray3_dbg.o amalloc_dbg.o areg_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

testdarray5_dbg: testdarray5_dbg.o amalloc_dbg.o areg_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

amalloc2dspeed_dbg: amalloc2dspeed_dbg.o \
	            amalloc2dspeed-auto_dbg.o \
                    amalloc2dspeed-exact_dbg.o \
                    amalloc2dspeed-dynamic_dbg.o \
                    amalloc2dspeed-amalloc_dbg.o \
                    amalloc_dbg.o areg_dbg.o pass.o test_damalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${LDLIBS}

amalloc2dspeed-dynamic_dbg.o: amalloc2dspeed-dynamic.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

amalloc2dspeed-auto_dbg.o: amalloc2dspeed-auto.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

amalloc2dspeed-amalloc_dbg.o: amalloc2dspeed-amalloc.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

amalloc2dspeed-exact_dbg.o: amalloc2dspeed-exact.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

amalloc_dbg.o: amalloc.c amalloc.h
	${CC} ${AMALLOCDBGCFLAGS} -c -o $@ $<

areg_dbg.o: areg.c areg.h
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
	\rm -f amalloc2dspeed-amalloc_dbg.o amalloc2dspeed-dynamic_dbg.o amalloc_dbg.o areg_dbg.o test_damalloc.o testdarray5_dbg.o amalloc2dspeed-amalloc.o amalloc2dspeed-dynamic.o amalloc.o testdarray2_dbg.o testdarray5.o amalloc2dspeed-auto_dbg.o amalloc2dspeed-exact_dbg.o darray.o testdarray2.o testdarray_dbg.o amalloc2dspeed-auto.o amalloc2dspeed-exact.o pass.o testdarray3_dbg.o testdarray.o amalloc2dspeed_dbg.o amalloc2dspeed.o test_damalloc_dbg.o testdarray3.o areg.o aregtest.o
