# Makefile for amalloc and its test programs

OBJDIR=obj
LIBDIR=lib

LDLIBS=-lm -lamalloc
LDFLAGS=-g -gdwarf-2 -O3 -pthread
CFLAGS=-Wall -g -gdwarf-2 -O3 -march=native
DBGLDFLAGS=-g -gdwarf-2 -pthread
DBGLDLIBS=-lm -lamalloc_dbg
DBGCFLAGS= -Wall -O0 -DDEBUG -g -gdwarf-2

AMALLOCCFLAGS=${CFLAGS} -DAREG_PTHREAD_LOCK -ansi -pedantic -finline-limit=256 
AMALLOCDBGCFLAGS=${DBGCFLAGS} -DAREG_PTHREAD_LOCK -ansi -pedantic

release: testdarray testdarray2 testdarray3 testdarray4 amalloc2dspeed testdarray5 aregtest test1d test2d test3d  

debug: testdarray_dbg testdarray2_dbg testdarray3_dbg amalloc2dspeed_dbg testdarray4_dbg testdarray5_dbg test1d_dbg test2d_dbg test3d_dbg aregtest_dbg

all: release debug

${OBJDIR}/amalloc.o: amalloc.c amalloc.h areg.ic
	${CC} ${AMALLOCCFLAGS} -fpic -c -o $@ $<

libamalloc.so.1.0: ${OBJDIR}/amalloc.o
	${CC} ${LDFLAGS} -shared  -Wl,-soname,libamalloc.so.1 -o $@ $^

libamalloc.so.1: libamalloc.so.1.0
	test -s libamalloc.so.1 || ln -s libamalloc.so.1.0 libamalloc.so.1

libamalloc.so: libamalloc.so.1
	test -s libamalloc.so || ln -s libamalloc.so.1 libamalloc.so

amalloc2dspeed: ${OBJDIR}/amalloc2dspeed.o \
                ${OBJDIR}/amalloc2dspeed-auto.o \
                ${OBJDIR}/amalloc2dspeed-exact.o \
                ${OBJDIR}/amalloc2dspeed-dynamic.o \
                ${OBJDIR}/amalloc2dspeed-amalloc.o \
                ${OBJDIR}/pass.o ${OBJDIR}/test_damalloc.o libamalloc.so
	${CC} ${LDFLAGS} -o $@ ${OBJDIR}/amalloc2dspeed.o ${OBJDIR}/amalloc2dspeed-*.o ${OBJDIR}/pass.o ${OBJDIR}/test_damalloc.o ${LDLIBS}

aregtest: ${OBJDIR}/aregtest.o 
	${CC} ${LDFLAGS} -o $@ $^ -lm

testdarray: ${OBJDIR}/testdarray.o libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray2: ${OBJDIR}/testdarray2.o libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray3: ${OBJDIR}/testdarray3.o libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray4: ${OBJDIR}/testdarray4.o libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray5: ${OBJDIR}/testdarray5.o libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

test1d: ${OBJDIR}/test1d.o libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

test2d: ${OBJDIR}/test2d.o libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

test3d: ${OBJDIR}/test3d.o libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${OBJDIR}/aregtest.o: areg.ic
	${CC} ${CFLAGS} -DAREG_PTHREAD_LOCK -DO_AREGTEST -x c -c -o $@ $<

${OBJDIR}/test_damalloc.o: test_damalloc.c test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $<

${OBJDIR}/testdarray.o: testdarray.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/testdarray2.o: testdarray2.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/testdarray3.o: testdarray3.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/testdarray4.o: testdarray4.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/testdarray5.o: testdarray5.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/test1d.o: test1d.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/test2d.o: test2d.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/test3d.o: test3d.c amalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/pass.o: pass.c
	${CC} -O0 -g -c -o $@ $<

${OBJDIR}/amalloc2dspeed.o: amalloc2dspeed.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/amalloc2dspeed-dynamic.o: amalloc2dspeed-dynamic.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/amalloc2dspeed-auto.o: amalloc2dspeed-auto.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/amalloc2dspeed-amalloc.o: amalloc2dspeed-amalloc.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJDIR}/amalloc2dspeed-exact.o: amalloc2dspeed-exact.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${CFLAGS} -c -o $@ $< 


${OBJDIR}/amalloc_dbg.o: amalloc.c amalloc.h areg.ic
	${CC} ${AMALLOCDBGCFLAGS} -fpic -c -o $@ $<

libamalloc_dbg.so.1.0: ${OBJDIR}/amalloc_dbg.o
	${CC} ${LDFLAGS} -shared  -Wl,-soname,libamalloc_dbg.so.1 -o libamalloc_dbg.so.1.0 ${OBJDIR}/amalloc_dbg.o

libamalloc_dbg.so.1: libamalloc_dbg.so.1.0
	test -s libamalloc_dbg.so.1  || ln -s libamalloc_dbg.so.1.0 libamalloc_dbg.so.1

libamalloc_dbg.so: libamalloc_dbg.so.1
	test -s libamalloc_dbg.so || ln -s libamalloc_dbg.so.1 libamalloc_dbg.so

aregtest_dbg: ${OBJDIR}/aregtest_dbg.o 
	${CC} ${DBGLDFLAGS} -o $@ $^ -lm

${OBJDIR}/aregtest_dbg.o: areg.ic
	${CC} ${DBGCFLAGS} -DAREG_PTHREAD_LOCK -DO_AREGTEST -x c -c -o $@ $<

testdarray_dbg: ${OBJDIR}/testdarray_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray2_dbg: ${OBJDIR}/testdarray2_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray3_dbg: ${OBJDIR}/testdarray3_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray4_dbg: ${OBJDIR}/testdarray4_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray5_dbg: ${OBJDIR}/testdarray5_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

test1d_dbg: ${OBJDIR}/test1d_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

test2d_dbg: ${OBJDIR}/test2d_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

test3d_dbg: ${OBJDIR}/test3d_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

amalloc2dspeed_dbg: ${OBJDIR}/amalloc2dspeed_dbg.o \
	            ${OBJDIR}/amalloc2dspeed-auto_dbg.o \
                    ${OBJDIR}/amalloc2dspeed-exact_dbg.o \
                    ${OBJDIR}/amalloc2dspeed-dynamic_dbg.o \
                    ${OBJDIR}/amalloc2dspeed-amalloc_dbg.o \
                    libamalloc_dbg.so ${OBJDIR}/pass.o ${OBJDIR}/test_damalloc_dbg.o
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

${OBJDIR}/amalloc2dspeed-dynamic_dbg.o: amalloc2dspeed-dynamic.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJDIR}/amalloc2dspeed-auto_dbg.o: amalloc2dspeed-auto.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJDIR}/amalloc2dspeed-amalloc_dbg.o: amalloc2dspeed-amalloc.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJDIR}/amalloc2dspeed-exact_dbg.o: amalloc2dspeed-exact.c amalloc.h cstopwatch.h test_damalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJDIR}/test_damalloc_dbg.o: test_damalloc.c test_damalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJDIR}/testdarray_dbg.o: testdarray.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJDIR}/testdarray2_dbg.o: testdarray2.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJDIR}/testdarray3_dbg.o: testdarray3.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJDIR}/testdarray4_dbg.o: testdarray4.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJDIR}/testdarray5_dbg.o: testdarray5.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJDIR}/test1d_dbg.o: test1d.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJDIR}/test2d_dbg.o: test2d.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJDIR}/test3d_dbg.o: test3d.c amalloc.h
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJDIR}/amalloc2dspeed_dbg.o: amalloc2dspeed.c amalloc.h  cstopwatch.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

clean:
	(cd ${OBJDIR} && \rm -f amalloc2dspeed-amalloc_dbg.o amalloc2dspeed-dynamic_dbg.o test_damalloc.o testdarray4_dbg.o testdarray4.o testdarray5_dbg.o amalloc2dspeed-amalloc.o amalloc2dspeed-dynamic.o amalloc.o testdarray2_dbg.o testdarray5.o amalloc2dspeed-auto_dbg.o amalloc2dspeed-exact_dbg.o darray.o testdarray2.o testdarray_dbg.o amalloc2dspeed-auto.o amalloc2dspeed-exact.o pass.o testdarray3_dbg.o testdarray.o amalloc2dspeed_dbg.o amalloc2dspeed.o test_damalloc_dbg.o testdarray3.o aregtest.o test1d.o test2d.o test3d.o test1d_dbg.o test2d_dbg.o test3d_dbg.o)
