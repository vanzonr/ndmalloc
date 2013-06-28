/*
 * amalloc.h
 *
 * Header file for dynamically allocated multi-dimensional c arrays of
 * arbitrary rank.
 *
 * Copyright (c) 2013 Ramses van Zon
 */

#ifndef _AMALLOC_RVZ_
#define _AMALLOC_RVZ_

#include <stddef.h>   /* for the definition of size_t */

/* 
 * Mimic the regular malloc/free/calloc/realloc set for allocating and
 * deallocating memory for multi-dimensional arrays.
 */
void* amalloc   (size_t size, long rank, ...);
void* acalloc   (size_t size, long rank, ...);
void* arealloc  (void* ptr, size_t size, long rank, ...);
void* aview     (void* data, size_t size, long rank, ...); 
void  afree     (void* ptr);

void* samalloc  (size_t size, long rank, const size_t* n);
void* sacalloc  (size_t size, long rank, const size_t* n);
void* sarealloc (void* ptr, size_t size, long rank, const size_t* n);
void* saview    (void* data, size_t size, long rank, const size_t* n); 

/*
 * Description:
 *
 *  The 'amalloc' function creates a dynamically allocated multi-
 *  dimensional array of dimensions n[0] x n[1] ... x n['rank'-1],
 *  with elements of 'size' bytes.  The dimensions are to be given as
 *  the variable-length arguments.  The function allocates
 *  'size'*n[0]*n[1]*..n['rank'-1] bytes for the data, plus another
 *  n[0]*n[1]*...n[rank-2]*sizeof(void*) bytes for the
 *  pointer-to-pointer structure that is common for c-style arrays.
 *  It also allocates some internal buffers.  The pointer-to-pointer
 *  structure assumes that all pointers are the same size.  The return
 *  value can be cast to a pointer of TYPE* for an array of rank 1,
 *  TYPE** for rank 2, T*** for rank 3, etc., where sizeof(TYPE) must
 *  equal the 'size' argument.  This casted pointer can then be used
 *  in the same way a c-style array is used, i.e., with repeated
 *  square bracket indexing.  If the memory allocation fails, a NULL
 *  pointer is returned.  The return value (or its casted version) can
 *  be used in calls to 'arealloc', 'afree', 'asize', 'afullsize',
 *  'adata', 'arank', 'ashape', and 'aknown'.  This works because an
 *  internal header containing the information about the
 *  multi-dimensional structure is associated with each dynamicaly
 *  allocated multi-dimensional array.
 *
 *  The 'afree' function frees up all the memory allocated for the
 *  multi-dimensional array associates with the pointer 'ptr'.
 *
 *  The 'aview' function is similar to malloc but only allocates the
 *  pointer-to pointer array, while the elements of the array should
 *  be a contiguous block pointed to by 'data'. 'rank' must be larger
 *  than one for a view to work.  'data' is allowed to point to the
 *  memory of another dynamically allocated array.  The pointer
 *  returned by 'aview' has to be freed with 'afree'.
 *
 *  The 'acalloc' function has the same functionality as 'amalloc',
 *  but also initializes the array to all zeros (by calling 'calloc').
 * 
 *  The 'arealloc' function chances the dimensions and/or the size of
 *  the multi-dimenstional array 'ptr'.  The content of the array
 *  will be unchanged in the range from the start of the region up to
 *  the minimum of the old and new sizes.  If the change in dimensions
 *  has changed the shape, the elements get reassigned indices
 *  according to the row-major ordering.  If the re-allocation is
 *  succesful, the new pointer is returned and the old one is invalid.
 *  If the function fails, NULL is returned.  Known bug: the original
 *  'ptr' is still deallocated when 'arealloc' fails.
 *
 *  The functions 'samalloc', 'sacalloc' and 'sarealloc' are
 *  non-variadic variants of 'amalloc', 'acalloc' and 'arealloc',
 *  respectively, taking instead an array of dimensions as the
 *  argument 'n'.
 */

/*
 * Functions to get information about the multi-dimensional arrays
 * allocated by amalloc, acalloc or arealloc.
 */
      int     aknown    (const void* ptr);
      int     aisview   (const void* ptr);
      long    arank     (const void* ptr);
      size_t  asize     (const void* ptr, long dim);
      size_t  afullsize (const void* ptr);
      void*   adata     (      void* ptr);
const void*   acdata    (const void* ptr);
const size_t* ashape    (const void* ptr);
/*
 * The function 'aknown' checks if 'ptr' is a 'known multi-dimensional
 * array', i.e., whether it was allocated using amalloc, acalloc, or
 * arealloc.  The return value is 1 if 'ptr' was successfully created
 * with amalloc, acalloc, arealloc, or aview (or their non-variadic
 * versions samalloc, sacalloc, sarealloc and saview), and 0 if it was
 * not.
 *
 * The function 'ais view' checks if 'ptr' is a 'known
 * multi-dimensional array view created with 'aview' or 'saview'.
 *
 * The function 'arank' * returns the rank of multi-dimensional array
 * 'ptr', or zero if 'ptr' is not a known multi-dimensional array.
 *
 * The function 'asize' returns the extent in the given dimension
 * 'dim', If 'ptr' is not a known multi-dimensional array, or if 'dim'
 * is larger than the rank of that array, 'asize' returns zero.
 *
 * The function 'afullsize' returns the total number of elements in
 * the multi-dimensional arrays (the product of all asize's). If 'ptr'
 * is not a known multi-dimensional array, 'afullsize' returns zero.
 *
 * The function 'adata' * returns the start of the data, or NULL if
 * 'ptr' is not a known multi-dimensional array.  The 'acdata' does
 * the same but a returns a const pointer, and can be used 
 * with a const pointer as an argument.
 *
 * The function 'ashape' returns a pointer the first element of an
 * array of sintegers which give the shape of the multi-dimensional
 * array, or NULL if 'ptr' is not a known multi-dimensional array.
 */


/*
 * Macros to turn automatic arrays into multi-dimensional views 
*/
#define autoview0(a) aview(a,sizeof(a),0))
#define autoview1(a) aview(a,sizeof(*(a)),1,sizeof(a)/sizeof(*(a)))
#define autoview2(a) aview(a,sizeof(**(a)),2,sizeof(a)/sizeof(*(a)),sizeof(*(a))/sizeof(**(a)))
#define autoview3(a) aview(a,sizeof(***(a)),3,sizeof(a)/sizeof(*(a)),sizeof(*(a))/sizeof(**(a)),sizeof(**(a))/sizeof(***(a)))
#define autoview4(a) aview(a,sizeof(****(a)),4,sizeof(a)/sizeof(*(a)),sizeof(*(a))/sizeof(**(a)),sizeof(**(a))/sizeof(***(a)),sizeof(***(a))/sizeof(****(a)))
#define autoview5(a) aview(a,sizeof(*****(a)),5,sizeof(a)/sizeof(*(a)),sizeof(*(a))/sizeof(**(a)),sizeof(**(a))/sizeof(***(a)),sizeof(***(a))/sizeof(****(a)),sizeof(****(a))/sizeof(*****(a)))
#define autoview6(a) aview(a,sizeof(******(a)),6,sizeof(*(a))/sizeof(**(a)),sizeof(**(a))/sizeof(***(a)),sizeof(***(a))/sizeof(****(a)),sizeof(****(a))/sizeof(*****(a)),sizeof(*****(a))/sizeof(******(a)))
#define autoview7(a) aview(a,sizeof(*******(a)),7,sizeof(**(a))/sizeof(***(a)),sizeof(***(a))/sizeof(****(a)),sizeof(****(a))/sizeof(*****(a)),sizeof(*****(a))/sizeof(******(a)),sizeof(******(a))/sizeof(*******(a)))
#define autoview8(a) aview(a,sizeof(********(a)),8,sizeof(***(a))/sizeof(****(a)),sizeof(****(a))/sizeof(*****(a)),sizeof(*****(a))/sizeof(******(a)),sizeof(******(a))/sizeof(*******(a)),sizeof(*******(a))/sizeof(********(a)))

#endif

/* end of file amalloc.h */
