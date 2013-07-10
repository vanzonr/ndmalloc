# Makefile for ndmalloc and its test programs

       OBJ=obj
       LIB=lib
    OBJTAG=obj/tag
    LIBTAG=lib/tag

    CFLAGS=-g -gdwarf-2 -O3 -Wall -march=native
 DBGCFLAGS=-g -gdwarf-2 -O0 -Wall -DDEBUG 
   LDFLAGS=-g -gdwarf-2 -pthread -L${LIB} -O3
DBGLDFLAGS=-g -gdwarf-2 -pthread -L${LIB} -O0
    LDLIBS=-lm -lndmalloc
 DBGLDLIBS=-lm -lndmalloc_dbg


   NDMALLOCCFLAGS=-DAREG_PTHREAD_LOCK -ansi -pedantic ${CFLAGS} -finline-limit=256 
NDMALLOCDBGCFLAGS=-DAREG_PTHREAD_LOCK -ansi -pedantic ${DBGCFLAGS}

#
# Meta-targets
#

.PHONY: release, release_lib, release_tst, \
        debug, debug_lib, debug_tst, \
        lib, all

release_lib: ${LIB}/libndmalloc.so

release_tst: testdarray testdarray2 testdarray3 testdarray4 ndmalloc2dspeed testdarray5 aregtest test1d test2d test3d  

release: release_lib release_tst

debug_lib: ${LIB}/libndmalloc_dbg.so

debug_tst: testdarray_dbg testdarray2_dbg testdarray3_dbg ndmalloc2dspeed_dbg testdarray4_dbg testdarray5_dbg test1d_dbg test2d_dbg test3d_dbg aregtest_dbg

debug: debug_lib debug_tst

lib: release_lib debug_lib

all: release debug

#                                                                      #
#  Directory structure                                                 # 
#                                                                      #

${OBJTAG}:
	mkdir -p ${OBJTAG}

${LIBTAG}:
	mkdir -p ${LIBTAG}

#                                                                      #
#  Release compilation                                                 #
#                                                                      #

${OBJ}/ndmalloc.o: ndmalloc.c ndmalloc.h areg.ic ${OBJTAG}
	${CC} ${NDMALLOCCFLAGS} -fpic -c -o $@ $<

${LIB}/libndmalloc.so.1.0: ${OBJ}/ndmalloc.o ${LIBTAG}
	${CC} ${LDFLAGS} -shared  -Wl,-soname,libndmalloc.so.1 -o $@ $<

${LIB}/libndmalloc.so.1: ${LIB}/libndmalloc.so.1.0
	test -s ${LIB}/libndmalloc.so.1 || ln -s libndmalloc.so.1.0 ${LIB}/libndmalloc.so.1

${LIB}/libndmalloc.so: ${LIB}/libndmalloc.so.1
	test -s ${LIB}/libndmalloc.so || ln -s libndmalloc.so.1 ${LIB}/libndmalloc.so

#                                                                      #
#  Release tests                                                       #
#                                                                      #

NDMALLOC2DSPEEDOBJS=${OBJ}/ndmalloc2dspeed.o \
                   ${OBJ}/ndmalloc2dspeed-auto.o \
                   ${OBJ}/ndmalloc2dspeed-exact.o \
                   ${OBJ}/ndmalloc2dspeed-dynamic.o \
                   ${OBJ}/ndmalloc2dspeed-ndmalloc.o \
                   ${OBJ}/pass.o \
	           ${OBJ}/test_damalloc.o

ndmalloc2dspeed: ${NDMALLOC2DSPEEDOBJS} ${LIB}/libndmalloc.so
	${CC} ${LDFLAGS} -o $@ ${NDMALLOC2DSPEEDOBJS} ${LDLIBS}

aregtest: ${OBJ}/aregtest.o 
	${CC} ${LDFLAGS} -o $@ $^ -lm

testdarray: ${OBJ}/testdarray.o ${LIB}/libndmalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray2: ${OBJ}/testdarray2.o ${LIB}/libndmalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray3: ${OBJ}/testdarray3.o ${LIB}/libndmalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray4: ${OBJ}/testdarray4.o ${LIB}/libndmalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

testdarray5: ${OBJ}/testdarray5.o ${LIB}/libndmalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

test1d: ${OBJ}/test1d.o ${LIB}/libndmalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

test2d: ${OBJ}/test2d.o ${LIB}/libndmalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

test3d: ${OBJ}/test3d.o ${LIB}/libndmalloc.so
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${OBJ}/aregtest.o: areg.ic ${OBJTAG}
	${CC} ${CFLAGS} -DAREG_PTHREAD_LOCK -DO_AREGTEST -x c -c -o $@ $<

${OBJ}/test_damalloc.o: test_damalloc.c test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $<

${OBJ}/testdarray.o: testdarray.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/testdarray2.o: testdarray2.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/testdarray3.o: testdarray3.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/testdarray4.o: testdarray4.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/testdarray5.o: testdarray5.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/test1d.o: test1d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/test2d.o: test2d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/test3d.o: test3d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/pass.o: pass.c ${OBJTAG}
	${CC} -O0 -g -c -o $@ $<

${OBJ}/ndmalloc2dspeed.o: ndmalloc2dspeed.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/ndmalloc2dspeed-dynamic.o: ndmalloc2dspeed-dynamic.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/ndmalloc2dspeed-auto.o: ndmalloc2dspeed-auto.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/ndmalloc2dspeed-ndmalloc.o: ndmalloc2dspeed-ndmalloc.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}/ndmalloc2dspeed-exact.o: ndmalloc2dspeed-exact.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

#                                                                      #
#   Debugging compilation                                              #
#                                                                      #

${OBJ}/ndmalloc_dbg.o: ndmalloc.c ndmalloc.h areg.ic ${OBJTAG}
	${CC} ${NDMALLOCDBGCFLAGS} -fpic -c -o $@ $<

${LIB}/libndmalloc_dbg.so.1.0: ${OBJ}/ndmalloc_dbg.o ${LIBTAG}
	${CC} ${LDFLAGS} -shared  -Wl,-soname,libndmalloc_dbg.so.1 -o ${LIB}/libndmalloc_dbg.so.1.0 ${OBJ}/ndmalloc_dbg.o

${LIB}/libndmalloc_dbg.so.1: ${LIB}/libndmalloc_dbg.so.1.0
	test -s ${LIB}/libndmalloc_dbg.so.1  || ln -s libndmalloc_dbg.so.1.0 ${LIB}/libndmalloc_dbg.so.1

${LIB}/libndmalloc_dbg.so: ${LIB}/libndmalloc_dbg.so.1
	test -s ${LIB}/libndmalloc_dbg.so || ln -s libndmalloc_dbg.so.1 ${LIB}/libndmalloc_dbg.so

#                                                                      #
#   Debugging tests                                                    #
#                                                                      #

aregtest_dbg: ${OBJ}/aregtest_dbg.o 
	${CC} ${DBGLDFLAGS} -o $@ $^ -lm

${OBJ}/aregtest_dbg.o: areg.ic ${OBJTAG}
	${CC} ${DBGCFLAGS} -DAREG_PTHREAD_LOCK -DO_AREGTEST -x c -c -o $@ $<

testdarray_dbg: ${OBJ}/testdarray_dbg.o ${LIB}/libndmalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray2_dbg: ${OBJ}/testdarray2_dbg.o ${LIB}/libndmalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray3_dbg: ${OBJ}/testdarray3_dbg.o ${LIB}/libndmalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray4_dbg: ${OBJ}/testdarray4_dbg.o ${LIB}/libndmalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

testdarray5_dbg: ${OBJ}/testdarray5_dbg.o ${LIB}/libndmalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

test1d_dbg: ${OBJ}/test1d_dbg.o ${LIB}/libndmalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

test2d_dbg: ${OBJ}/test2d_dbg.o ${LIB}/libndmalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

test3d_dbg: ${OBJ}/test3d_dbg.o ${LIB}/libndmalloc_dbg.so
	${CC} ${DBGLDFLAGS} -o $@ $^ ${DBGLDLIBS}

NDMALLOC2DSPEEDDBGOBJS=${OBJ}/ndmalloc2dspeed_dbg.o \
	              ${OBJ}/ndmalloc2dspeed-auto_dbg.o \
                      ${OBJ}/ndmalloc2dspeed-exact_dbg.o \
                      ${OBJ}/ndmalloc2dspeed-dynamic_dbg.o \
                      ${OBJ}/ndmalloc2dspeed-ndmalloc_dbg.o \
                      ${OBJ}/pass.o ${OBJ}/test_damalloc_dbg.o 

ndmalloc2dspeed_dbg: ${NDMALLOC2DSPEEDDBGOBJS} ${LIB}/libndmalloc_dbg.so 
	${CC} ${DBGLDFLAGS} -o $@ ${NDMALLOC2DSPEEDDBGOBJS} ${DBGLDLIBS}

${OBJ}/ndmalloc2dspeed-dynamic_dbg.o: ndmalloc2dspeed-dynamic.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}/ndmalloc2dspeed-auto_dbg.o: ndmalloc2dspeed-auto.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}/ndmalloc2dspeed-ndmalloc_dbg.o: ndmalloc2dspeed-ndmalloc.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}/ndmalloc2dspeed-exact_dbg.o: ndmalloc2dspeed-exact.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}/test_damalloc_dbg.o: test_damalloc.c test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/testdarray_dbg.o: testdarray.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/testdarray2_dbg.o: testdarray2.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/testdarray3_dbg.o: testdarray3.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/testdarray4_dbg.o: testdarray4.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/testdarray5_dbg.o: testdarray5.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/test1d_dbg.o: test1d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/test2d_dbg.o: test2d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/test3d_dbg.o: test3d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}/ndmalloc2dspeed_dbg.o: ndmalloc2dspeed.c ndmalloc.h  cstopwatch.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

clean:
	\rm -f ${NDMALLOC2DSPEEDDBGOBJS} ${NDMALLOC2DSPEEDOBJS}
	(cd ${OBJ} && \rm -f testdarray4_dbg.o testdarray4.o testdarray5_dbg.o ndmalloc.o testdarray2_dbg.o testdarray5.o testdarray2.o testdarray_dbg.o testdarray3_dbg.o testdarray.o test_damalloc_dbg.o testdarray3.o aregtest.o test1d.o test2d.o test3d.o test1d_dbg.o test2d_dbg.o test3d_dbg.o ndmalloc_dbg.o aregtest_dbg.o)
