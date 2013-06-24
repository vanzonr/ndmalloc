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
    char*restrict* ptr = calloc(n0,sizeof(char*));
    int i;
    ptr[0] = malloc(size*n1*n0);
    for (i=0;i<n0;i++)
      //  ptr[i] = malloc(size*n1);
     ptr[i] = ptr[0] + i*n1*size;
    return (void*)ptr;
}
void* test_dacalloc2d(size_t size, size_t n0, size_t n1){
    char** ptr = calloc(n0,sizeof(char*));
    int i;
    for (i=0;i<n0;i++)
        ptr[i] = calloc(n1,size);
    return ptr;
}
void  test_dafree2d(void* ptr, size_t n0){
    char** theptr = ptr;
    int i;
    //for (i=0;i<n0;i++)    
    i=0;
        free(theptr[i]);
    free(ptr);
}

/* end of file test_damalloc.hc */
