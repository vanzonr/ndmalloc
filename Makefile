# Makefile for amalloc and its test programs

       OBJ=obj
       LIB=lib
    OBJTAG=obj/tag
    LIBTAG=lib/tag

    CFLAGS=-g -gdwarf-2 -O3 -Wall -march=native
 DBGCFLAGS=-g -gdwarf-2 -O0 -Wall -DDEBUG 
   LDFLAGS=-g -gdwarf-2 -pthread -L${LIB} -O3
DBGLDFLAGS=-g -gdwarf-2 -pthread -L${LIB} -O0
    LDLIBS=-lm -lamalloc
 DBGLDLIBS=-lm -lamalloc_dbg


AMALLOCCFLAGS=${CFLAGS} -DAREG_PTHREAD_LOCK -ansi -pedantic -finline-limit=256 
AMALLOCDBGCFLAGS=${DBGCFLAGS} -DAREG_PTHREAD_LOCK -ansi -pedantic

release: testdarray testdarray2 testdarray3 testdarray4 amalloc2dspeed testdarray5 aregtest test1d test2d test3d  

debug: testdarray_dbg testdarray2_dbg testdarray3_dbg amalloc2dspeed_dbg testdarray4_dbg testdarray5_dbg test1d_dbg test2d_dbg test3d_dbg aregtest_dbg

all: release debug

#                                                              #
#  Directory structure                                         # 
#                                                              #
${OBJTAG}:
	mkdir -p ${OBJTAG}

${LIBTAG}:
	mkdir -p ${LIBTAG}

${OBJ}/amalloc.o: amalloc.c amalloc.h areg.ic ${OBJTAG}
	${CC} ${AMALLOCCFLAGS} -fpic -c -o $@ $<

${LIB}/libamalloc.so.1.0: ${OBJ}/amalloc.o ${LIBTAG}
	${CC} ${LDFLAGS} -shared  -Wl,-soname,libamalloc.so.1 -o $@ $<

${LIB}/libamalloc.so.1: ${LIB}/libamalloc.so.1.0
	test -s ${LIB}/libamalloc.so.1 || ln -s libamalloc.so.1.0 ${LIB}/libamalloc.so.1

${LIB}/libamalloc.so: ${LIB}/libamalloc.so.1
	test -s ${LIB}/libamalloc.so || ln -s libamalloc.so.1 ${LIB}/libamalloc.so

AMALLOC2DSPEEDOBJS=${OBJ}/amalloc2dspeed.o \
                   ${OBJ}/amalloc2dspeed-auto.o \
                   ${OBJ}/amalloc2dspeed-exact.o \
                   ${OBJ}/amalloc2dspeed-dynamic.o \
                   ${OBJ}/amalloc2dspeed-amalloc.o \
                   ${OBJ}/pass.o \
	           ${OBJ}/test_damalloc.o

amalloc2dspeed: ${AMALLOC2DSPEEDOBJS} ${LIB}/libamalloc.so
	${CC} ${LDFLAGS} -o $@ ${AMALLOC2DSPEEDOBJS} ${LDLIBS}

aregtest: ${OBJ}/aregtest.o 
	${CC} ${LDFLAGS} -o $@ $^ -lm

testdarray: ${OBJ}/testdarray.o ${LIB}/libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray2: ${OBJ}/testdarray2.o ${LIB}/libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray3: ${OBJ}/testdarray3.o ${LIB}/libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray4: ${OBJ}/testdarray4.o ${LIB}/libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray5: ${OBJ}/testdarray5.o ${LIB}/libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

test1d: ${OBJ}/test1d.o ${LIB}/libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

test2d: ${OBJ}/test2d.o ${LIB}/libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

test3d: ${OBJ}/test3d.o ${LIB}/libamalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${OBJ}/aregtest.o: areg.ic ${OBJTAG}
	${CC} ${CFLAGS} -DAREG_PTHREAD_LOCK -DO_AREGTEST -x c -c -o $@ $<

${OBJ}/test_damalloc.o: test_damalloc.c test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $<

${OBJ}/testdarray.o: testdarray.c amalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/testdarray2.o: testdarray2.c amalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/testdarray3.o: testdarray3.c amalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/testdarray4.o: testdarray4.c amalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/testdarray5.o: testdarray5.c amalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/test1d.o: test1d.c amalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/test2d.o: test2d.c amalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/test3d.o: test3d.c amalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/pass.o: pass.c ${OBJTAG}
	${CC} -O0 -g -c -o $@ $<

${OBJ}/amalloc2dspeed.o: amalloc2dspeed.c amalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/amalloc2dspeed-dynamic.o: amalloc2dspeed-dynamic.c amalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/amalloc2dspeed-auto.o: amalloc2dspeed-auto.c amalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/amalloc2dspeed-amalloc.o: amalloc2dspeed-amalloc.c amalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/amalloc2dspeed-exact.o: amalloc2dspeed-exact.c amalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

#                                                             #
#   For debugging:                                            #
#                                                             #

${OBJ}/amalloc_dbg.o: amalloc.c amalloc.h areg.ic ${OBJTAG}
	${CC} ${AMALLOCDBGCFLAGS} -fpic -c -o $@ $<

${LIB}/libamalloc_dbg.so.1.0: ${OBJ}/amalloc_dbg.o ${LIBTAG}
	${CC} ${LDFLAGS} -shared  -Wl,-soname,libamalloc_dbg.so.1 -o ${LIB}/libamalloc_dbg.so.1.0 ${OBJ}/amalloc_dbg.o

${LIB}/libamalloc_dbg.so.1: ${LIB}/libamalloc_dbg.so.1.0
	test -s ${LIB}/libamalloc_dbg.so.1  || ln -s libamalloc_dbg.so.1.0 ${LIB}/libamalloc_dbg.so.1

${LIB}/libamalloc_dbg.so: ${LIB}/libamalloc_dbg.so.1
	test -s ${LIB}/libamalloc_dbg.so || ln -s libamalloc_dbg.so.1 ${LIB}/libamalloc_dbg.so

aregtest_dbg: ${OBJ}/aregtest_dbg.o 
	${CC} ${DBGLDFLAGS} -o $@ $^ -lm

${OBJ}/aregtest_dbg.o: areg.ic ${OBJTAG}
	${CC} ${DBGCFLAGS} -DAREG_PTHREAD_LOCK -DO_AREGTEST -x c -c -o $@ $<

testdarray_dbg: ${OBJ}/testdarray_dbg.o ${LIB}/libamalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray2_dbg: ${OBJ}/testdarray2_dbg.o ${LIB}/libamalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray3_dbg: ${OBJ}/testdarray3_dbg.o ${LIB}/libamalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray4_dbg: ${OBJ}/testdarray4_dbg.o ${LIB}/libamalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray5_dbg: ${OBJ}/testdarray5_dbg.o ${LIB}/libamalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

test1d_dbg: ${OBJ}/test1d_dbg.o ${LIB}/libamalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

test2d_dbg: ${OBJ}/test2d_dbg.o ${LIB}/libamalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

test3d_dbg: ${OBJ}/test3d_dbg.o ${LIB}/libamalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

AMALLOC2DSPEEDDBGOBJS=${OBJ}/amalloc2dspeed_dbg.o \
	              ${OBJ}/amalloc2dspeed-auto_dbg.o \
                      ${OBJ}/amalloc2dspeed-exact_dbg.o \
                      ${OBJ}/amalloc2dspeed-dynamic_dbg.o \
                      ${OBJ}/amalloc2dspeed-amalloc_dbg.o \
                      ${OBJ}/pass.o ${OBJ}/test_damalloc_dbg.o 
amalloc2dspeed_dbg: ${AMALLOC2DSPEEDDBGOBJS} ${LIB}/libamalloc_dbg.so 
	${CC} ${DBGLDFLAGS} -o $@ ${AMALLOC2DSPEEDDBGOBJS} ${DBGLDLIBS}

${OBJ}/amalloc2dspeed-dynamic_dbg.o: amalloc2dspeed-dynamic.c amalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}/amalloc2dspeed-auto_dbg.o: amalloc2dspeed-auto.c amalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}/amalloc2dspeed-amalloc_dbg.o: amalloc2dspeed-amalloc.c amalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}/amalloc2dspeed-exact_dbg.o: amalloc2dspeed-exact.c amalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}/test_damalloc_dbg.o: test_damalloc.c test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/testdarray_dbg.o: testdarray.c amalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/testdarray2_dbg.o: testdarray2.c amalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/testdarray3_dbg.o: testdarray3.c amalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/testdarray4_dbg.o: testdarray4.c amalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/testdarray5_dbg.o: testdarray5.c amalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/test1d_dbg.o: test1d.c amalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/test2d_dbg.o: test2d.c amalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/test3d_dbg.o: test3d.c amalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/amalloc2dspeed_dbg.o: amalloc2dspeed.c amalloc.h  cstopwatch.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

clean:
	(cd ${OBJ} && \rm -f amalloc2dspeed-amalloc_dbg.o amalloc2dspeed-dynamic_dbg.o test_damalloc.o testdarray4_dbg.o testdarray4.o testdarray5_dbg.o amalloc2dspeed-amalloc.o amalloc2dspeed-dynamic.o amalloc.o testdarray2_dbg.o testdarray5.o amalloc2dspeed-auto_dbg.o amalloc2dspeed-exact_dbg.o darray.o testdarray2.o testdarray_dbg.o amalloc2dspeed-auto.o amalloc2dspeed-exact.o pass.o testdarray3_dbg.o testdarray.o amalloc2dspeed_dbg.o amalloc2dspeed.o test_damalloc_dbg.o testdarray3.o aregtest.o test1d.o test2d.o test3d.o test1d_dbg.o test2d_dbg.o test3d_dbg.o)
