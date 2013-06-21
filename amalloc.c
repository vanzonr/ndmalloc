/*
 * amalloc.c
 *
 * Implementation of the amalloc library, which can make arbitrary
 * multi-dimensional c arrays. 
 *
 * Copyright (c) 2013 Ramses van Zon
 */

#include "amalloc.h"
#include <stdlib.h>
#include <stdarg.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* 
 * Define a header struct, with all the information about the
 * dynamically allocated array, except the pointer-to-pointer.  This
 * header will be placed before the addresses holdinng the data and
 * array pointers. A special magic_mark is added to the rank field, as
 * a check that we do indeed have a dynamically allocated array.
 */
typedef struct {
    void*   data;         /* Pointer to the contiguous elements     */
    size_t  size;         /* How big is each element?               */
    size_t  rank;         /* #dimensions, but with magic_mark added */
    size_t* shape;        /* What are those dimensions?             */
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
header_t* da__get_header_address( void* array )
{
    return (header_t*)((char*)array - header_size);
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
 * Internal function to prepend the dynamic pointer with a header 
 */
static 
void da__create( void*    array,
                 void*    data,
                 size_t   size,
                 size_t   rank,
                 size_t*  shape )
{  
    header_t* hdr = da__get_header_address(array);
    hdr->data  = data;
    hdr->size  = size;
    hdr->rank  = (rank & magic_unmask) | magic_mark;
    hdr->shape = shape;
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
void* da__vamalloc( size_t   size, 
                    size_t   rank, 
                    va_list  arglist )
{
    void*    array;
    void*    data;
    size_t*  shape;
    size_t   i, total_elements;

    shape = malloc(sizeof(size_t)*rank);
    if (shape == NULL) 
        return NULL;
    
    for (i = 0; i < rank; i++) 
        shape[i] = va_arg(arglist, size_t);
  
    total_elements = 1;
    for (i = 0; i < rank; i++) 
        total_elements *= shape[i];

    data = malloc(total_elements*size + header_size);
    if (data == NULL) {
        free(shape);
        return NULL;
    }
    data = (char*)data + header_size;
    
    array = da__pointer_structure(data, size, rank, shape);
    if (array == NULL) {
        free(shape);
        free((char*)data - header_size);
        return NULL;
    } else {
        da__create(array, data, size, rank, shape);
        return array;
    }
}

/*
 * Internal function to allocate and clear memory with dimensions from a va_list
 */
static 
void* da__vacalloc( size_t   size, 
                    size_t   rank, 
                    va_list  arglist )
{
    void*    array;
    void*    data;
    size_t*  shape;
    size_t   i, total_elements;
    
    shape = malloc(sizeof(size_t)*rank);
    if (shape == NULL) 
        return NULL;
    
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
        return NULL;
    }
    data = (char*)data + header_size;

    array = da__pointer_structure(data, size, rank, shape);
    if (array == NULL) {
        free(shape);
        free((char*)data - header_size);
        return NULL;
    } else {
        da__create(array, data, size, rank, shape);
        return array;
    }
}

/*
 * Internal function to reallocate (or reshape!) with dimensions from a va_list
 */
static 
void* da__varealloc( void*    ptr, 
                     size_t   size, 
                     size_t   rank, 
                     va_list  arglist )
{
    void*      array;
    void*      data;
    size_t*    shape;
    size_t     i, total_elements, oldrank;
    header_t*  hdr;
    
    hdr = da__get_header_address(ptr);
    data = hdr->data;
    shape = hdr->shape;
    oldrank = hdr->rank;

    free(shape);
    if (oldrank > 1) 
       free((char*)ptr - header_size);

    shape = malloc(sizeof(size_t)*rank);
    if (shape == NULL) 
        return NULL;
    
    for (i = 0; i < rank; i++) 
      shape[i] = va_arg(arglist, size_t);

    total_elements = 1;
    for (i = 0; i < rank; i++) 
        total_elements *= shape[i];

    data = realloc((char*)data - header_size, 
                   total_elements*size + header_size);
    if (data == NULL) {
        free(shape);
        return NULL;
    }
    data = (char*)data + header_size;

    array = da__pointer_structure(data, size, rank, shape);
    if (array == NULL) {
        free(shape);
        free((char*)data - header_size);
        return NULL;
    } 
    else {
        da__create(array, data, size, rank, shape);
        return array;
    }
}


/*
 * IMPLEMENTATION OF THE INTERFACE
 */


/*
 *  The 'amalloc' function creates a dynamically allocated multi-
 *  dimensional array of dimensions n[0] x n[1] ... x n['rank'-1],
 *  with elements of 'size' bytes.  The dimensions are to be given as
 *  the variable-length arguments.  The function allocates
 *  'size'*n[0]*n[1]*..n['rank'-1] bytes for the data, plus another
 *  n[0]*n[1]*...n[rank-2]*sizeof(void*) bytes for the
 *  pointer-to-pointer structure that is common for c-style arrays.
 *  It also allocates internal buffers of moderate size (~64 bytes).
 *  The pointer-to-pointer structure assumes that all pointers are the
 *  same size.  The return value can be cast to a TYPE* for an array
 *  of rank 1, TYPE** for rank 2, T*** for rank 3, etc.  .This casted
 *  pointer can then be used in the same way a c-style array is used,
 *  i.e., with repeated square bracket indexing.  If the memory
 *  allocation fails, a NULL pointer is returned.  The return value
 *  (or its casted version) can be used in calls to 'arealloc',
 *  'afree', 'asize', 'adata', 'arank', 'ashape', 'aknown', and
 *  'atoda'.  This works because an internal header containing the
 *  information about the multi-dimensional structure is associated
 *  with each dynamicaly allocated multi-dimensional array.
 */
void* amalloc(size_t size, size_t rank, ...)
{
    void* result;
    va_list arglist;
    va_start(arglist, rank);
    result = da__vamalloc(size, rank, arglist);
    va_end(arglist);
    return result;
}

/*
 *  The 'acalloc' function has the same functionality as amalloc, but
 *  also initialized the array to all zeros (by calling 'calloc').
 */
void* acalloc(size_t size, size_t rank, ...)
{
    void* result;
    va_list arglist;
    va_start(arglist, rank);
    result = da__vacalloc(size, rank, arglist);
    va_end(arglist);
    return result;
}

/*
 *  The 'arealloc' function chances the dimensions and/or the size of
 *  the multi-dimenstional array 'ptr'.  The content of the array
 *  will be unchanged in the range from the start of the region up to
 *  the minimum of the old and new sizes.  If the change in dimensions
 *  has changed the shape, the elements get reassigned indices
 *  according to the row-major ordering.  If the re-allocation is
 *  succesful, the new pointer is returned and the old one is invalid.
 *  If the function fails, NULL is returned.  Known bug: the original
 *  'ptr' is still deallocated when 'arealloc' fails.
 */
void* arealloc(void* ptr, size_t size, size_t rank, ...)
{
    void* result;
    va_list arglist;
    va_start(arglist, rank);
    result = da__varealloc(ptr, size, rank, arglist);
    va_end(arglist);
    return result;
}

/*
 *  The 'afree' function frees up all the memory allocated for the
 *  multi-dimensional array associates with the pointer 'ptr'.
 */
void afree(void* ptr)
{
    header_t* hdr = da__get_header_address(ptr);
    if (da__is_header(hdr)) {
        free(hdr->shape);
        free((char*)hdr->data - header_size);
        if (hdr->rank > 1) 
           free((char*)ptr - header_size);
    }
}

/*
 * Function to get the extent in any given dimension. 
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.
 */
size_t asize(void* ptr, size_t dim)
{
    header_t* hdr = da__get_header_address(ptr);
    if (da__is_header(hdr)) {
        if (dim < hdr->rank)
            return hdr->shape[dim];
        else
            return 1;
    } else
        return 0;
}

/*
 * Function to get the start of the data (useful for library calls).
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.  Returns NULL if
 * 'ptr' was not allocated with amalloc, acalloc, or arealloc.
 */
void* adata(void* ptr)
{
    header_t* hdr = da__get_header_address(ptr);
    if (da__is_header(hdr))
        return hdr->data;
    else
        return NULL;
}

/*
 * Function to get the rank of the multi-dimensional array.
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.  Returns 0 if
 * 'ptr' was not allocated with amalloc, acalloc, or arealloc.
 */
size_t arank(void* ptr)
{
    header_t* hdr = da__get_header_address(ptr);
    if (da__is_header(hdr))
        return hdr->rank & magic_unmask;
    else
        return 0;
}

/*
 * Function to get the shape of the multi-dimensional array.  This
 * works because an internal header containing the information about
 * the multi-dimensional structure is associated with each dynamicaly
 * allocated multi-dimensional array. Returns NULL if no dynamic array
 * is associated with 'ptr'.
 */
const size_t* ashape(void* ptr)
{
    header_t* hdr = da__get_header_address(ptr);
    if (da__is_header(hdr))
        return hdr->shape;
    else
        return 0;
}

/* 
 * Function to check if 'ptr's is an array allocated with amalloc,
 * acalloc, or arealloc.  This works because an internal header
 * containing the information about the multi-dimensional structure is
 * associated with each dynamicaly allocated multi-dimensional array.
 */
int aknown(void* ptr)
{
    header_t* hdr = da__get_header_address(ptr);
    return da__is_header(hdr);
}

/* end of file amalloc.c */
