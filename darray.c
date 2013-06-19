/*
 * darray.c
 *
 * Implementation of the darray library, which can make arbitrary
 * multi-dimensional c arrays. 
 *
 * Copyright (c) 2013 Ramses van Zon
 */

#include "darray.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* 
 * Define a partial darray struct, just enough to be able to convert
 * say a double** to a darray.  This header will be placed before the
 * addresses holdinng the data and array pointers in the darray
 * struct. A special magic_mark is added to the rank field,
 * as a check that we do indeed have a darray struct. 
 */
typedef struct {
    void*   data;    /* as in darray */
    size_t  size;    /* as in darray */
    size_t  rank;    /* as in darray, but with magic_mark added */
    size_t* shape;   /* as in darray */
} header_t;

/* 
 * Define the magic mark to be embedded in rank, as well as some
 * useful masks
 */
#define magic_mark       0x19720000
#define magic_mask       0xffff0000
#define magic_unmask     0x0000ffff

/* 
 * Define an alignment policy, such that the headers and the actual
 * data are a multiple of mem_align_bytes apart.
 */
#define mem_align_x      4
#define mem_align_bytes  (mem_align_x*sizeof(char*))
#define header_size      (((sizeof(header_t)+mem_align_bytes-1) \
                          /mem_align_bytes)*mem_align_bytes)
#define header_ptr_size  (header_size/sizeof(char*))



/*
 * INTERNAL ROUTINES
 *
 * (start with 'da__' and are static)                                     
 *
 */


/*
 * Internal function to get the hidden header given the pointer-to-pointer array
 */
static 
header_t* da__get_header( void* array )
{
    return (header_t*)((char*)array - header_size);
}

/*
 * Internal function to mark a given header with the correct magic stamp 
 */
static 
void da__mark_header( header_t* hdr )
{
    hdr->rank &= magic_unmask;
    hdr->rank |= magic_mark;
}

/*
 * Internal function to check that a given header has the correct magic stamp 
 */
static 
int da__is_header( header_t* hdr )
{
    return (hdr->rank & magic_mask) == magic_mark;
}

/*
 * Internal function to create darray struct and prepend it with a header 
 */
static 
darray da__create( void*    array,
                   void*    data,
                   size_t   size,
                   size_t   rank,
                   size_t*  shape )
{  
    header_t* hdr = da__get_header(array);
    hdr->data  = data;
    hdr->size  = size;
    hdr->rank  = rank;
    hdr->shape = shape;
    da__mark_header(hdr);
    return (darray){array, data, size, rank, shape};
}

/*
 * Internal function to create the pointer-to-pointer structure for any rank 
 */
static 
void* da__pointer_structure( void*    data,
                             size_t   size,
                             size_t   rank,
                             size_t*  shape )
{
    if (rank <= 1) 

        return data;
       
    else {
        
        size_t  i, j, ntot;
        size_t  nalloc;    
        char**  palloc;
        char**  result;
        char*** ptr;
        
        nalloc = 0;
        for (i = rank-1; i--; )
            nalloc = shape[i]*(1+nalloc);
        
        palloc = (char**)calloc(nalloc + header_ptr_size, sizeof(char*));
        if (palloc == NULL)
            return NULL;
        palloc += header_ptr_size;
        
        ntot = 1;   
        ptr = &result;
        for (i = 0; i < rank - 1; i++) {    
            for (j = 0; j < ntot; j++)
                ptr[j] = palloc + j*shape[i];      
            ptr = (char***)(*ptr);
            ntot *= shape[i];
            palloc += ntot;
        }
        for (j = 0; j < ntot; j++)
            ptr[j] = (char**)((char*)data 
                              + size*j*shape[rank-1]);
        return (void*)result;
    }
}

/*
 * Internal function routine to allocate the memory with indices from a va_list
 */
static 
darray da__vdmalloc( size_t   size, 
                     size_t   rank, 
                     va_list  arglist )
{
    void*    array;
    void*    data;
    size_t*  shape;
    size_t   i, total_elements;

    shape = malloc(sizeof(size_t)*rank);
    if (shape == NULL) 
        return DNULL;
    
    for (i = 0; i < rank; i++) 
        shape[i] = va_arg(arglist, size_t);
  
    total_elements = 1;
    for (i = 0; i < rank; i++) 
        total_elements *= shape[i];

    data = malloc(total_elements*size + header_size);
    if (data == NULL) {
        free(shape);
        return DNULL;
    }
    data = (char*)data + header_size;
    
    array = da__pointer_structure(data, size, rank, shape);
    if (array == NULL) {
        free(shape);
        free((char*)data - header_size);
        return DNULL;
    } else 
        return da__create(array, data, size, rank, shape);
}

/*
 * Internal function to allocate and clear memory with dimensions from a va_list
 */
static 
darray da__vdcalloc( size_t   size, 
                     size_t   rank, 
                     va_list  arglist)
{
    void*    array;
    void*    data;
    size_t*  shape;
    size_t   i, total_elements;
    
    shape = malloc(sizeof(size_t)*rank);
    if (shape == NULL) 
        return DNULL;
    
    for (i = 0; i < rank; i++) 
        shape[i] = va_arg(arglist, size_t);
    
    total_elements = 1;
    for (i = 0; i < rank; i++) 
        total_elements *= shape[i];
    
    size_t chunks = (total_elements*size+header_size+mem_align_bytes-1)
                    /mem_align_bytes;
    data = calloc(chunks, mem_align_bytes);
    if (data == NULL) {
        free(shape);
        return DNULL;
    }
    data = (char*)data + header_size;

    array = da__pointer_structure(data, size, rank, shape);
    if (array == NULL) {
        free(shape);
        free((char*)data - header_size);
        return DNULL;
    } else
        return da__create(array, data, size, rank, shape);
}

/*
 * Internal function to reallocate (or reshape!) with dimensions from a va_list
 */
static 
darray da__vdrealloc( darray   darr, 
                      size_t   size, 
                      size_t   rank, 
                      va_list  arglist )
{
    void*    array;
    void*    data;
    size_t*  shape;
    size_t   i, total_elements;

    free(darr.shape);
    free((char*)darr.array + header_size);

    shape = malloc(sizeof(size_t)*rank);
    if (shape == NULL) 
        return DNULL;
    
    for (i = 0; i < rank; i++) 
      shape[i] = va_arg(arglist, size_t);

    total_elements = 1;
    for (i = 0; i < rank; i++) 
        total_elements *= shape[i];

    data = realloc((char*)darr.data - header_size, 
                   total_elements*size + header_size);
    if (data == NULL) {
        free(shape);
        return DNULL;
    }
    data = (char*)data + header_size;

    array = da__pointer_structure(data, size, rank, shape);
    if (array == NULL) {
        free(shape);
        free((char*)data - header_size);
        return DNULL;
    } 
    else
        return da__create(array, data, size, rank, shape);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 


/*
 * IMPLEMENTATION OF THE API
 */

/*
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
 */
darray damalloc( size_t size, 
                size_t rank, 
                ... )
{
    darray result;
    va_list arglist;
    va_start(arglist, rank);
    #ifdef __TINYC__
    // tcc stdarg is off by one element in struct returning functions
    va_arg(arglist, size_t);
    #endif
    result = da__vdmalloc(size, rank, arglist);
    va_end(arglist);
    return result;
}

/* 
 *  The 'dacalloc' function has the same functionality as damalloc, but
 *  also initializes the array content to zero (it does so by calling
 *  'calloc').
 */ 
darray dacalloc(size_t size, size_t rank, ...)
{
    darray result;
    va_list arglist;
    va_start(arglist, rank);
    #ifdef __TINYC__
    // tcc stdarg is off by one element in struct returning functions
    va_arg(arglist, size_t);
    #endif
    result = da__vdcalloc(size, rank, arglist);
    va_end(arglist);
    return result;
}

/*
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
darray darealloc(darray darr, size_t size, size_t rank, ...)
{
    darray result;
    va_list arglist;
    va_start(arglist, rank);
    #ifdef __TINYC__
    // tcc stdarg is off by one element in struct returning functions
    va_arg(arglist, size_t);
    #endif
    result = da__vdrealloc(darr, size, rank, arglist);
    va_end(arglist);
    return result;
}

/* 
 *  The 'dafree' function frees up all the memory allocated for the
 *  multi-dimensional structure 'darr'. The 'darr' structure is
 *  invalid after the call.
 */
void dafree(darray darr)
{  
    free(darr.shape);
    free((char*)darr.data - header_size);
    if (darr.rank > 1) 
      free((char*)darr.array - header_size);
}

/* 
 * Function to get the extent in any given dimension. 
 */
size_t daextentof(darray darr, size_t dim)
{
    return (dim < darr.rank) ? darr.shape[dim] : 0;
}

/*
 * Function to check whether darr == DNULL 
 */
int danotnull(darray darr)
{
    return darr.array != NULL || darr.data != NULL || darr.shape != NULL
        || darr.rank != 0 || darr.size != 0;
}

/*
 * Function to get the start of the data (useful for library calls).
 */
void* dadata(darray darr)
{
    return darr.data;
}

/*
 * Function to get the rank of the multi-dimensional array.
 */
size_t darank(darray darr)
{
    return darr.rank;
}

/*
 * Function to get the shape of the multi-dimensional array
 */
const size_t* dashape(darray darr)
{
    return darr.shape;
}

/*
 * Accessor/modifier functions (... is a list of integer indices).
 * Included for completion, converting to a TYPE** will be much
 * faster.xx 
 */
void daget(void* x, darray a, ...)
{
    size_t   i, n;
    va_list  arglist;
    char**   start;
    void*    y;
    
    start = a.array;
    va_start(arglist, a);
    for (i = 0; i < a.rank-1; i++) {
        n = va_arg(arglist, size_t);
        start = (char**)(start[n]);
    }
    n = va_arg(arglist, size_t);
    va_end(arglist);
    
    y = (void*)((char*)start + n*a.size);
    
    memcpy(x, y, a.size);  
}

void daset(void* x, darray a, ...)
{
    size_t   i, n;
    va_list  arglist;
    char**   start;
    void*    y;
    
    start = a.array;
    va_start(arglist, a);
    for (i = 0; i < a.rank-1; i++) {
        n = va_arg(arglist, size_t);
        start = (char**)(start[n]);
    }
    n = va_arg(arglist, size_t);
    va_end(arglist);
    
    y = (void*)((char*)start + n*a.size);
    
    memcpy(y, x, a.size); 
}

/* 
 *  Convert to TYPE** or similar 
 */
void* datopa(darray darr)
{
    return darr.array;
}

/* 
 *  Convert back from type** to a darray 
 */
darray patoda(void* arr)
{
    header_t* hdr = da__get_header(arr);
    if (da__is_header(hdr)) {
        return (darray) {
            arr, 
            hdr->data, 
            hdr->size, 
            hdr->rank & magic_unmask, 
            hdr->shape
        };
    } 
    else 
        return DNULL;
}

/*
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
 */
void* pamalloc(size_t size, size_t rank, ...)
{
    darray result;
    va_list arglist;
    va_start(arglist, rank);
    result = da__vdmalloc(size, rank, arglist);
    va_end(arglist);
    if ( danotnull(result) )
        return result.array;
    else
        return NULL;
}

/*
 *  The 'pacalloc' function has the same functionality as pamalloc, but
 *  also initialized the array to all zeros (by calling 'calloc').
 */
void* pacalloc(size_t size, size_t rank, ...)
{
    darray result;
    va_list arglist;
    va_start(arglist, rank);
    result = da__vdcalloc(size, rank, arglist);
    va_end(arglist);
    if ( danotnull(result) )
        return result.array;
    else
        return NULL;
}

/*
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
void* parealloc(void* arr, size_t size, size_t rank, ...)
{
    darray result;
    va_list arglist;
    va_start(arglist, rank);
    result = da__vdrealloc(patoda(arr), size, rank, arglist);
    va_end(arglist);
    if ( danotnull(result) )
        return result.array;
    else
        return NULL;
}

/*
 *  The 'pafree' function frees up all the memory allocated for the
 *  multi-dimensional array associates with the pointer 'arr'.
 */
void pafree(void* arr)
{
  darray darr = patoda(arr);
  if ( danotnull(darr) )
    dafree(darr);
}

/*
 * Function to get the extent in any given dimension. 
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.
 */
size_t paextentof(void* arr, size_t dim)
{
    darray darr = patoda(arr);
    if ( danotnull(darr) )
        return daextentof(darr, dim);
    else
        return 0;
}

/*
 * Function to get the start of the data (useful for library calls).
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.  Returns NULL if no
 * darray is associated with arr.
 */
void* padata(void* arr)
{
    header_t* hdr = da__get_header(arr);
    if (da__is_header(hdr))
        return hdr->data;
    else
        return 0;
}

/*
 * Function to get the rank of the multi-dimensional array.
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.  Returns 0 if no
 * darray is associated with arr.
 */
size_t parank(void* arr)
{
    header_t* hdr = da__get_header(arr);
    if (da__is_header(hdr))
        return hdr->rank;
    else
        return 0;
}

/*
 * Function to get the shape of the multi-dimensional array This works
 * because an internal header containing the information about the
 * multi-dimensional structure is associated with each dynamicaly
 * allocated multi-dimensional array. Returns NULL if no darray is
 * associated with arr.
 */
const size_t* pashape(void* arr)
{
    header_t* hdr = da__get_header(arr);
    if (da__is_header(hdr))
        return hdr->shape;
    else
        return 0;
}

/* 
 * Function to check if this is a darray. 
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.
 */
int paisda(void* arr)
{
    header_t* hdr = da__get_header(arr);
    return da__is_header(hdr);
}

/* end of file darray.c */
