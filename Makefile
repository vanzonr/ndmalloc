# Makefile for ndmalloc and its test programs

       BIN=bin/
       OBJ=obj/
       LIB=lib/
    BINTAG=bin/tag
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

release_lib: ${LIB}libndmalloc.so

release_tst: ${BIN}testc2d ${BIN}testd2d ${BIN}testb2d ${BIN}testc3d ${BIN}ndmalloc2dspeed ${BIN}testb3d ${BIN}aregtest ${BIN}testa1d ${BIN}testa2d ${BIN}testa3d  

release: release_lib release_tst

debug_lib: ${LIB}libndmalloc_dbg.so

debug_tst: ${BIN}testc2d_dbg ${BIN}testd2d_dbg ${BIN}testb2d_dbg ${BIN}ndmalloc2dspeed_dbg ${BIN}testc3d_dbg ${BIN}testb3d_dbg ${BIN}testa1d_dbg ${BIN}testa2d_dbg ${BIN}testa3d_dbg ${BIN}aregtest_dbg

debug: debug_lib debug_tst

lib: release_lib debug_lib

all: release debug

#                                                                      #
#  Directory structure                                                 # 
#                                                                      #

${BINTAG}:
	mkdir -p ${BINTAG}

${OBJTAG}:
	mkdir -p ${OBJTAG}

${LIBTAG}:
	mkdir -p ${LIBTAG}

#                                                                      #
#  Release compilation                                                 #
#                                                                      #

${OBJ}ndmalloc.o: ndmalloc.c ndmalloc.h areg.ic ${OBJTAG}
	${CC} ${NDMALLOCCFLAGS} -fpic -c -o $@ $<

${LIB}libndmalloc.so.1.0: ${OBJ}ndmalloc.o ${LIBTAG}
	${CC} ${LDFLAGS} -shared  -Wl,-soname,libndmalloc.so.1 -o $@ $<

${LIB}libndmalloc.so.1: ${LIB}libndmalloc.so.1.0
	test -s ${LIB}libndmalloc.so.1 || ln -s libndmalloc.so.1.0 ${LIB}libndmalloc.so.1

${LIB}libndmalloc.so: ${LIB}libndmalloc.so.1
	test -s ${LIB}libndmalloc.so || ln -s libndmalloc.so.1 ${LIB}libndmalloc.so

#                                                                      #
#  Release tests                                                       #
#                                                                      #

NDMALLOC2DSPEEDOBJS=${OBJ}ndmalloc2dspeed.o \
                   ${OBJ}ndmalloc2dspeed-auto.o \
                   ${OBJ}ndmalloc2dspeed-exact.o \
                   ${OBJ}ndmalloc2dspeed-dynamic.o \
                   ${OBJ}ndmalloc2dspeed-ndmalloc.o \
                   ${OBJ}pass.o \
	           ${OBJ}test_damalloc.o

${BIN}ndmalloc2dspeed: ${NDMALLOC2DSPEEDOBJS} ${LIB}libndmalloc.so ${BINTAG}
	${CC} ${LDFLAGS} -o $@ ${NDMALLOC2DSPEEDOBJS} ${LDLIBS}

${BIN}aregtest: ${OBJ}aregtest.o ${BINTAG}
	${CC} ${LDFLAGS} -o $@ $< -lm

${BIN}testc2d: ${OBJ}testc2d.o ${LIB}libndmalloc.so ${BINTAG}
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${BIN}testd2d: ${OBJ}testd2d.o ${LIB}libndmalloc.so ${BINTAG}
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${BIN}testb2d: ${OBJ}testb2d.o ${LIB}libndmalloc.so ${BINTAG}
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${BIN}testc3d: ${OBJ}testc3d.o ${LIB}libndmalloc.so ${BINTAG}
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${BIN}testb3d: ${OBJ}testb3d.o ${LIB}libndmalloc.so ${BINTAG}
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${BIN}testa1d: ${OBJ}testa1d.o ${LIB}libndmalloc.so ${BINTAG}
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${BIN}testa2d: ${OBJ}testa2d.o ${LIB}libndmalloc.so ${BINTAG}
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${BIN}testa3d: ${OBJ}testa3d.o ${LIB}libndmalloc.so ${BINTAG}
	${CC} ${LDFLAGS} -o $@ $< ${LDLIBS}

${OBJ}aregtest.o: areg.ic ${OBJTAG}
	${CC} ${CFLAGS} -DAREG_PTHREAD_LOCK -DO_AREGTEST -x c -c -o $@ $<

${OBJ}test_damalloc.o: test_damalloc.c test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $<

${OBJ}testc2d.o: testc2d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}testd2d.o: testd2d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}testb2d.o: testb2d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}testc3d.o: testc3d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}testb3d.o: testb3d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}testa1d.o: testa1d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}testa2d.o: testa2d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}testa3d.o: testa3d.c ndmalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}pass.o: pass.c ${OBJTAG}
	${CC} -O0 -g -c -o $@ $<

${OBJ}ndmalloc2dspeed.o: ndmalloc2dspeed.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}ndmalloc2dspeed-dynamic.o: ndmalloc2dspeed-dynamic.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}ndmalloc2dspeed-auto.o: ndmalloc2dspeed-auto.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}ndmalloc2dspeed-ndmalloc.o: ndmalloc2dspeed-ndmalloc.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

${OBJ}ndmalloc2dspeed-exact.o: ndmalloc2dspeed-exact.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${CFLAGS} -c -o $@ $< 

#                                                                      #
#   Debugging compilation                                              #
#                                                                      #

${OBJ}ndmalloc_dbg.o: ndmalloc.c ndmalloc.h areg.ic ${OBJTAG}
	${CC} ${NDMALLOCDBGCFLAGS} -fpic -c -o $@ $<

${LIB}libndmalloc_dbg.so.1.0: ${OBJ}ndmalloc_dbg.o ${LIBTAG}
	${CC} ${LDFLAGS} -shared  -Wl,-soname,libndmalloc_dbg.so.1 -o ${LIB}libndmalloc_dbg.so.1.0 ${OBJ}ndmalloc_dbg.o

${LIB}libndmalloc_dbg.so.1: ${LIB}libndmalloc_dbg.so.1.0
	test -s ${LIB}libndmalloc_dbg.so.1  || ln -s libndmalloc_dbg.so.1.0 ${LIB}libndmalloc_dbg.so.1

${LIB}libndmalloc_dbg.so: ${LIB}libndmalloc_dbg.so.1
	test -s ${LIB}libndmalloc_dbg.so || ln -s libndmalloc_dbg.so.1 ${LIB}libndmalloc_dbg.so

#                                                                      #
#   Debugging tests                                                    #
#                                                                      #

${BIN}aregtest_dbg: ${OBJ}aregtest_dbg.o ${BINTAG}
	${CC} ${DBGLDFLAGS} -o $@ $< -lm

${OBJ}aregtest_dbg.o: areg.ic ${OBJTAG}
	${CC} ${DBGCFLAGS} -DAREG_PTHREAD_LOCK -DO_AREGTEST -x c -c -o $@ $<

${BIN}testc2d_dbg: ${OBJ}testc2d_dbg.o ${LIB}libndmalloc_dbg.so ${BINTAG}
	${CC} ${DBGLDFLAGS} -o $@ $< ${DBGLDLIBS}

${BIN}testd2d_dbg: ${OBJ}testd2d_dbg.o ${LIB}libndmalloc_dbg.so ${BINTAG}
	${CC} ${DBGLDFLAGS} -o $@ $< ${DBGLDLIBS}

${BIN}testb2d_dbg: ${OBJ}testb2d_dbg.o ${LIB}libndmalloc_dbg.so ${BINTAG}
	${CC} ${DBGLDFLAGS} -o $@ $< ${DBGLDLIBS}

${BIN}testc3d_dbg: ${OBJ}testc3d_dbg.o ${LIB}libndmalloc_dbg.so ${BINTAG}
	${CC} ${DBGLDFLAGS} -o $@ $< ${DBGLDLIBS}

${BIN}testb3d_dbg: ${OBJ}testb3d_dbg.o ${LIB}libndmalloc_dbg.so ${BINTAG}
	${CC} ${DBGLDFLAGS} -o $@ $< ${DBGLDLIBS}

${BIN}testa1d_dbg: ${OBJ}testa1d_dbg.o ${LIB}libndmalloc_dbg.so ${BINTAG}
	${CC} ${DBGLDFLAGS} -o $@ $< ${DBGLDLIBS}

${BIN}testa2d_dbg: ${OBJ}testa2d_dbg.o ${LIB}libndmalloc_dbg.so ${BINTAG}
	${CC} ${DBGLDFLAGS} -o $@ $< ${DBGLDLIBS}

${BIN}testa3d_dbg: ${OBJ}testa3d_dbg.o ${LIB}libndmalloc_dbg.so ${BINTAG}
	${CC} ${DBGLDFLAGS} -o $@ $< ${DBGLDLIBS}

NDMALLOC2DSPEEDDBGOBJS=${OBJ}ndmalloc2dspeed_dbg.o \
	              ${OBJ}ndmalloc2dspeed-auto_dbg.o \
                      ${OBJ}ndmalloc2dspeed-exact_dbg.o \
                      ${OBJ}ndmalloc2dspeed-dynamic_dbg.o \
                      ${OBJ}ndmalloc2dspeed-ndmalloc_dbg.o \
                      ${OBJ}pass.o ${OBJ}test_damalloc_dbg.o 

${BIN}ndmalloc2dspeed_dbg: ${NDMALLOC2DSPEEDDBGOBJS} ${LIB}libndmalloc_dbg.so ${BINTAG}
	${CC} ${DBGLDFLAGS} -o $@ ${NDMALLOC2DSPEEDDBGOBJS} ${DBGLDLIBS}

${OBJ}ndmalloc2dspeed-dynamic_dbg.o: ndmalloc2dspeed-dynamic.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}ndmalloc2dspeed-auto_dbg.o: ndmalloc2dspeed-auto.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}ndmalloc2dspeed-ndmalloc_dbg.o: ndmalloc2dspeed-ndmalloc.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}ndmalloc2dspeed-exact_dbg.o: ndmalloc2dspeed-exact.c ndmalloc.h cstopwatch.h test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $< 

${OBJ}test_damalloc_dbg.o: test_damalloc.c test_damalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}testc2d_dbg.o: testc2d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}testd2d_dbg.o: testd2d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}testb2d_dbg.o: testb2d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}testc3d_dbg.o: testc3d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}testb3d_dbg.o: testb3d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}testa1d_dbg.o: testa1d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}testa2d_dbg.o: testa2d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}testa3d_dbg.o: testa3d.c ndmalloc.h ${OBJTAG}
	${CC} ${DBGCFLAGS} -c -o $@ $<

${OBJ}ndmalloc2dspeed_dbg.o: ndmalloc2dspeed.c ndmalloc.h  cstopwatch.h
	${CC} ${DBGCFLAGS} -c -o $@ $< 

clean:
	\rm -f ${NDMALLOC2DSPEEDDBGOBJS} ${NDMALLOC2DSPEEDOBJS}
	(cd ${OBJ} && \rm -f testc3d_dbg.o testc3d.o testb3d_dbg.o ndmalloc.o testd2d_dbg.o testb3d.o testd2d.o testc2d_dbg.o testb2d_dbg.o testc2d.o test_damalloc_dbg.o testb2d.o aregtest.o testa1d.o testa2d.o testa3d.o testa1d_dbg.o testa2d_dbg.o testa3d_dbg.o ndmalloc_dbg.o aregtest_dbg.o)
