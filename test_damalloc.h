/*
 * test_damalloc.h
 *
 * Header file for dynamically allocated multi-dimensional c arrays of
 * rank two, for comparison with ndmalloc.
 *
 * Copyright (c) 2013 Ramses van Zon
 */

#ifndef _TESTDAMALLOC_RVZ_
#define _TESTDAMALLOC_RVZ_
#include <stddef.h>   /* for the definition of size_t */
void*  test_damalloc2d(size_t size, size_t n0, size_t n1);
void*  test_dacalloc2d(size_t size, size_t n0, size_t n1);
void*  test_darealloc2d(void* ptr, size_t size, size_t n0, size_t n1);
void   test_dafree2d(void* ptr, size_t n0); /* note we need the first dim */
#endif

/* end of file test_damalloc.h */
