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

#include <stddef.h>   /* for the definition of size_t */


/*
 *
 *  FIRST INTERFACE USING DARRAY STRUCTURE
 *
 */


/* 
 *  Define the structure 
 */
typedef struct 
{
    void*   array;        /* Pointer-to-pointer structure       */
    void*   data;         /* Pointer to the contiguous elements */
    size_t  size;         /* How big is each element?           */
    size_t  rank;         /* How many dimensions are there?     */
    size_t* shape;        /* What are those dimensions?         */

} darray;

/* 
 *  Define a darray constant that takes the place of the NULL pointer 
 */
#define DNULL ((darray){ NULL, NULL, 0, 0, NULL })

/* 
 * Mimic the regular malloc/free/calloc/realloc set, but with darray
 * instead of void*: (Note: ... is a list of integer extents)
 */
darray damalloc(size_t size, size_t rank, ...);
darray dacalloc(size_t size, size_t rank, ...);
darray darealloc(darray darr, size_t size, size_t rank, ...);
void   dafree(darray darr);
/*
 * Description:
 * 
 *  The 'damalloc' function creates a dynamically allocated multi-
 *  dimensional array of dimensions n[0] x n[1] ... x n[rank-1].  The
 *  dimensions are to be given as the variable-length arguments.  The
 *  function allocates 'size'*n[0]*n[1]*..n['rank'-1] bytes for the
 *  data, plus another n[0]*n[1]*...n[rank-2]*sizeof(void*) bytes for
 *  the pointer-to-pointer structure that is common for c-style
 *  arrays.  It also allocates internal buffers of currently at most
 *  2**sizeof('darray').  The pointer-to-pointer structure assumes
 *  that all pointers are the same size and that the elements of the
 *  array are of size 'size'.  It returns a 'darray', which is a
 *  structure that contains the information about the multi-
 *  dimensional structure.  This structure can be used in calls to
 *  'darealloc', 'dafree', 'daextentof', 'danotnull', 'dadata',
 *  'darank', 'dashape', 'daget', 'daset', and 'datopa'.  If the
 *  memory allocation fails, a special darray structure called DNULL
 *  is returned.
 *
 *  The 'dafree' function frees up all the memory allocated for the
 *  multi-dimensional structure 'darr'. The 'darr' structure is
 *  invalid after the call.
 *
 *  The 'dacalloc' function has the same functionality as 'damalloc',
 *  but also initializes the array to zero (by calling 'calloc').
 * 
 *  The 'darealloc' function chances the dimensions and/or the size of
 *  the multi-dimenstional array 'darr'.  The content of the array
 *  will be unchanged in the range from the start of the region up to
 *  the minimum of the old and new sizes.  If the change in dimensions
 *  has changed the shape, the elements get reassigned indices
 *  according to the row-major ordering.  If the re-allocation is
 *  succesful, the new darray is returned and the 'darr' is no longer
 *  valid.  If the function fails, DNULL is returned.  Known bug: the
 *  original 'darr' is still invalidated when 'darealloc' fails.
 */

/* 
 * Function to get the extent in any given dimension. 
 */
size_t daextentof(darray darr, size_t dim);

/*
 * Function to check whether darr == DNULL 
 */
int danotnull(darray darr);

/*
 * Function to get the start of the data (useful for library calls).
 */
void* dadata(darray darr);

/*
 * Function to get the rank of the multi-dimensional array.
 */
size_t darank(darray darr);

/*
 * Function to get the shape of the multi-dimensional array
 */
const size_t* dashape(darray darr);

/*
 * Accessor/modifier functions (... is a list of integer indices).
 * Included for completion, converting to a TYPE** will be much
 * faster.xx 
 */
void daget(void* x, darray a, ...);
void daset(void* x, darray a, ...);


/*
 *
 * SECOND INTERFACE USING void*S
 *
 */


/* 
 * Mimic the regular malloc/free/calloc/realloc set.
 */
void* pamalloc(size_t size, size_t rank, ...);
void* pacalloc(size_t size, size_t rank, ...);
void* parealloc(void* arr, size_t size, size_t rank, ...);
void  pafree(void* arr);
/*
 * Description:
 *
 *  The 'pamalloc' function creates a dynamically allocated multi-
 *  dimensional array of dimensions n[0] x n[1] ... x n['rank'-1],
 *  with elements of 'size' bytes.  The dimensions are to be given as
 *  the variable-length arguments.  The function allocates
 *  'size'*n[0]*n[1]*..n['rank'-1] bytes for the data, plus another
 *  n[0]*n[1]*...n[rank-2]*sizeof(void*) bytes for the
 *  pointer-to-pointer structure that is common for c-style arrays.
 *  It also allocates internal buffers of currently at most
 *  2**sizeof('darray').  The pointer-to-pointer structure assumes
 *  that all pointers are the same size.  The return value can be cast
 *  to a TYPE* for an array of rank 1, TYPE** for rank 2, T*** for
 *  rank 3, etc.  .This casted pointer can then be used in the same
 *  way a c-style array is used, i.e., with repeated square bracket
 *  indexing.  If the memory allocation fails, a NULL pointer is
 *  returned.  The return value (or its casted version) can be used in
 *  calls to 'parealloc', 'pafree', 'paextentof', 'padata', 'parank',
 *  'pashape', 'paisda', and 'patoda'.  This works because an internal
 *  header containing the information about the multi-dimensional
 *  structure is associated with each dynamicaly allocated
 *  multi-dimensional array.
 *
 *  The 'pafree' function frees up all the memory allocated for the
 *  multi-dimensional array associates with the pointer 'arr'.
 *
 *  The 'pacalloc' function has the same functionality as 'pamalloc',
 *  but also initializes the array to all zeros (by calling 'calloc').
 * 
 *  The 'parealloc' function chances the dimensions and/or the size of
 *  the multi-dimenstional array 'arr'.  The content of the array
 *  will be unchanged in the range from the start of the region up to
 *  the minimum of the old and new sizes.  If the change in dimensions
 *  has changed the shape, the elements get reassigned indices
 *  according to the row-major ordering.  If the re-allocation is
 *  succesful, the new pointer is returned and the old one is invalid.
 *  If the function fails, NULL is returned.  Known bug: the original
 *  'arr' is still deallocated when 'parealloc' fails.
 */

/*
 * Function to get the extent in any given dimension. 
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.
 */
size_t paextentof(void* arr, size_t dim);

/*
 * Function to get the start of the data (useful for library calls).
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.  Returns NULL if no
 * darray is associated with arr.
 */
void* padata(void* arr);

/*
 * Function to get the rank of the multi-dimensional array.  This
 * works because an internal header containing the information about
 * the multi-dimensional structure is associated with each dynamicaly
 * allocated multi-dimensional array.  Returns 0 if no darray is
 * associated with arr.
 */
size_t parank(void* arr);

/*
 * Function to get the shape of the multi-dimensional array This works
 * because an internal header containing the information about the
 * multi-dimensional structure is associated with each dynamicaly
 * allocated multi-dimensional array. Returns NULL if no darray is
 * associated with arr.
 */
const size_t* pashape(void* arr);

/* 
 * Function to check if this is a darray. 
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.
 */
int paisda(void* arr);


/*
 *
 *  CONVERSIONS FROM FIRST TO SECOND INTERFACE
 *
 */


/* 
 *  Convert to TYPE** or similar 
 */
void* datopa(darray darr);

/* 
 *  Convert back from type** to a darray 
 */
darray patoda(void* arr);

#endif

/* end of file darray.h */
