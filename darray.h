/*
 * darray.h
 *
 * Header file for dynamically allocated multi-dimensional c arrays of
 * arbitrary rank.
 *
 * Copyright (c) 2013 Ramses van Zon
 */

#ifndef _DARRAYH_
#define _DARRAYH_

#include <stddef.h>  /* for size_t */


/*     *     *     *     *     *     *     *     *\
 
 *    FIRST INTERFACE USING DARRAY STRUCTURE     *

 *     *     *     *     *     *     *     *     */


/* Define the structure */
typedef struct {
  void*   array;        /* Pointer-to-pointer structure       */
  void*   data;         /* Pointer to the contiguous elements */
  size_t  size;         /* How big is each element?           */
  size_t  rank;         /* How many dimensions are there?     */
  size_t* shape;        /* What are those dimensions?         */
} darray;

/* Define a darray constant that takes the place of the NULL pointer */
#define DNULL ((darray){ NULL, NULL, 0, 0, NULL })

/* Mimic the regular malloc/free/calloc/realloc set, but with darray
   instead of void*: (Note: ... is a list of integer extents) */
darray dmalloc(size_t size, size_t rank, ...);
darray dcalloc(size_t size, size_t rank, ...);
darray drealloc(darray darr, size_t size, size_t rank, ...);
void   dfree(darray darr);

/* Function to get the extent in any given dimension. */
size_t dextentof(darray darr, size_t dim);

/* Function to check whether darr == DNULL */
int dnotnull(darray darr);

/* Accessor/modifier functions (... is a list of integer indices).  
   Included for completion, converting to a TYPE** will be much faster. */
void dget(void* x, darray a, ...);
void dset(void* x, darray a, ...);


/*     *     *     *     *     *     *\

 *   SECOND INTERFACE USING void*S   *

\*     *     *     *     *     *     */


/* Mimic the regular malloc/free/calloc/realloc set */
void* amalloc(size_t size, size_t rank, ...);
void* acalloc(size_t size, size_t rank, ...);
void* arealloc(void* arr, size_t size, size_t rank, ...);
void  afree(void* arr);

/* Function to get the extent in any given dimension. */
size_t aextentof(void* arr, size_t dim);

/* Function to check if this is a darray. */
int aisd(void* arr);


/*     *     *     *     *     *     *     *     *\

 *  Conversions from first to second interface.  *

\*     *     *     *     *     *     *     *     */


/* Convert to type** or similar */
void* dtoa(darray darr);

/* Convert back from type** to a darray */
darray atod(void* arr);

#endif
