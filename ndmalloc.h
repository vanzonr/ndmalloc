/*
 * ndmalloc.h
 *
 * Header file for dynamically allocated multi-dimensional c arrays of
 * arbitrary rank.
 *
 * Copyright (c) 2013 Ramses van Zon
 */

#ifndef _NDMALLOC_RVZ_
#define _NDMALLOC_RVZ_

#include <stddef.h>   /* defines size_t */

/* Mimic the regular malloc/free/calloc/realloc functions for
 * allocating and deallocating memory, but for multi-dimensional
 * arrays. 
 */
void* ndmalloc   (size_t size, short rank, ...);
void* ndcalloc   (size_t size, short rank, ...);
void* ndrealloc  (void* ptr, size_t size, short rank, ...);
void* ndview     (void* data, size_t size, short rank, ...); 
void  ndfree     (void* ptr);
/* */
void* sndmalloc  (size_t size, short rank, const size_t* n);
void* sndcalloc  (size_t size, short rank, const size_t* n);
void* sndrealloc (void* ptr, size_t size, short rank, const size_t* n);
void* sndview    (void* data, size_t size, short rank, const size_t* n); 
/* Description:
 *  The 'ndmalloc' function creates a dynamically allocated multi-
 *  dimensional array of dimensions n[0] x n[1] ... x n['rank'-1],
 *  with elements of 'size' bytes.  The dimensions are to be given as
 *  the variable-length arguments '...'.  The function allocates
 *  'size'*n[0]*n[1]*..n['rank'-1] bytes for the data, plus another
 *  n[0]*n[1]*...n[rank-2]*sizeof(void*) bytes for the
 *  pointer-to-pointer structure that is common for c-style arrays.
 *  It also allocates about 24 bytes of internal buffers.  The
 *  pointer-to-pointer structure assumes that all pointers are the
 *  same size.  The return value can be cast to a pointer of TYPE* for
 *  an array of rank 1, TYPE** for rank 2, T*** for rank 3, etc.,
 *  where sizeof(TYPE) must equal the 'size' argument.  This casted
 *  pointer can then be used in the same way a c-style array is used,
 *  i.e., with repeated square bracket indexing.  If the memory
 *  allocation fails, a NULL pointer is returned.  The return value
 *  (or its casted version) can be used in calls to 'ndrealloc',
 *  'ndfree', 'ndsize', 'ndfullsize', 'nddata', 'ndcdata, 'ndrank', 'ndshape', and
 *  'ndisknown'.  This works because an internal header containing the
 *  information about the multi-dimensional structure is associated
 *  with each dynamicaly allocated multi-dimensional array.
 *
 *  The 'ndfree' function frees up all the memory allocated for the
 *  multi-dimensional array associated with the pointer 'ptr'.
 *
 *  The 'ndview' function is similar to ndmalloc but only allocates the
 *  pointer-to pointer array, while the elements of the array should
 *  be a contiguous block pointed to by 'data'. 'rank' must be larger
 *  than one for a view to work.  'data' is allowed to point to the
 *  memory of another dynamically allocated array.  The pointer
 *  returned by 'ndview' has to be freed with 'ndfree'.
 *
 *  The 'ndcalloc' function has the same functionality as 'ndmalloc',
 *  but also initializes the array to zero (by calling 'calloc').
 * 
 *  The 'ndrealloc' function changes the dimensions and/or the size of
 *  the multi-dimenstional array 'ptr'.  The content of the array will
 *  be unchanged in the range from the start of the region up to the
 *  minimum of the old and new sizes.  If the change in dimensions has
 *  changed the shape, the elements get reassigned indices according
 *  to the row-major ordering.  If the re-allocation is successful,
 *  the new pointer is returned and the old one is invalid.  If the
 *  function fails, NULL is returned.  Known bug: the original 'ptr'
 *  is still deallocated when 'ndrealloc' fails.
 *
 *  The functions 'sndmalloc', 'sndcalloc' and 'sndrealloc' are
 *  non-variadic variants of 'ndmalloc', 'acalloc' and 'arealloc',
 *  respectively, taking instead an array of dimensions as the
 *  argument 'n'.
 */

/* Functions to get information about the multi-dimensional arrays
 * allocated by ndmalloc, ndcalloc or ndrealloc.
 */
      int     ndisknown  (const void* ptr);
      int     ndisview   (const void* ptr);
      short   ndrank     (const void* ptr);
      size_t  ndsize     (const void* ptr, short dim);
      size_t  ndfullsize (const void* ptr);
      void*   nddata     (      void* ptr);
const void*   ndcdata    (const void* ptr);
const size_t* ndshape    (const void* ptr);
/* Descriptions:
 *  The function 'ndisknown' checks if 'ptr' is a 'known multi-dimensional
 *  array', i.e., whether it was allocated using ndmalloc, ndcalloc, or
 *  ndrealloc.  The return value is 1 if 'ptr' was successfully created
 *  with ndmalloc, ndcalloc, ndrealloc, or ndview (or their non-variadic
 *  versions sndmalloc, sndcalloc, sndrealloc and sndview), and 0 if it was
 *  not.
 *
 *  The function 'ndisview' checks if 'ptr' is a known
 *  multi-dimensional array view created with 'ndview' or 'sndview'.
 *
 *  The function 'ndrank'  returns the rank of multi-dimensional array
 *  'ptr'. If 'ptr' is not a known multi-dimensional array, the result
 *  is undefined.
 *
 *  The function 'ndsize' returns the extent in the given dimension
 *  'dim'. If 'ptr' is not a known multi-dimensional array, the result
 *  is undefined.
 *
 *  The function 'ndfullsize' returns the total number of elements in
 *  the multi-dimensional arrays (the product of all ndsize's). If 'ptr'
 *  is not a known multi-dimensional array, the result is undefined.
 *
 *  The function 'nddata' returns the start of the data. The result
 *  is undefined if 'ptr' is not a known multi-dimensional array.  The
 *  function 'ndcdata' does the same but a returns a const pointer, and
 *  can be used with a const pointer as an argument.
 *
 *  The function 'ndshape' returns a pointer the first element of an
 *  array of integers which give the shape of the multi-dimensional
 *  array. The result is undefined if 'ptr' is not a known
 *  multi-dimensional array.
 */


/* Macros to turn automatic arrays into multi-dimensional views */
#define autoview2(a)  ndview(a,sizeof(**(a)),2,sizeof(a)/sizeof(*(a)),sizeof(*(a))/sizeof(**(a)))
#define autoview3(a)  ndview(a,sizeof(***(a)),3,sizeof(a)/sizeof(*(a)),sizeof(*(a))/sizeof(**(a)),sizeof(**(a))/sizeof(***(a)))
#define autoview4(a)  ndview(a,sizeof(****(a)),4,sizeof(a)/sizeof(*(a)),sizeof(*(a))/sizeof(**(a)),sizeof(**(a))/sizeof(***(a)),sizeof(***(a))/sizeof(****(a)))
#define autoview5(a)  ndview(a,sizeof(*****(a)),5,sizeof(a)/sizeof(*(a)),sizeof(*(a))/sizeof(**(a)),sizeof(**(a))/sizeof(***(a)),sizeof(***(a))/sizeof(****(a)),sizeof(****(a))/sizeof(*****(a)))
#define autoview6(a)  ndview(a,sizeof(******(a)),6,sizeof(a)/sizeof(*(a)),sizeof(*(a))/sizeof(**(a)),sizeof(**(a))/sizeof(***(a)),sizeof(***(a))/sizeof(****(a)),sizeof(****(a))/sizeof(*****(a))),sizeof(*****(a))/sizeof(******(a)))
#define autoview7(a)  ndview(a,sizeof(*******(a)),7,sizeof(a)/sizeof(*(a)),sizeof(*(a))/sizeof(**(a)),sizeof(**(a))/sizeof(***(a)),sizeof(***(a))/sizeof(****(a)),sizeof(****(a))/sizeof(*****(a))),sizeof(*****(a))/sizeof(******(a))),sizeof(******(a))/sizeof(*******(a)))

#endif

