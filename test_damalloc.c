/*
 * test_damalloc.c
 *
 * Dynamically allocated multi-dimensional c arrays of
 * rank two, for comparison with amalloc.
 *
 * Copyright (c) 2013 Ramses van Zon
 */
#include "test_damalloc.h"
#include <stdlib.h>
void* test_damalloc2d(size_t size, size_t n0, size_t n1){
   char** ptr = malloc(sizeof(char*)*n0);
   int i;
   for (i=0;i<n0;i++)
      ptr[i] = malloc(size*n1);
   return ptr;
}
void* test_dacalloc2d(size_t size, size_t n0, size_t n1){
   char** ptr = malloc(sizeof(char*)*n0);
   int i;
   for (i=0;i<n0;i++)
      ptr[i] = calloc(n1,size);
   return ptr;
}
void  test_dafree2d(void* ptr, size_t n0){
   char** theptr = ptr;
   int i;
   for (i=0;i<n0;i++)
      free(theptr[i]);
   free(ptr);
}

/* end of file test_damalloc.hc */
