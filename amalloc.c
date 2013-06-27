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
 * array pointers. A special magic_mark is added, as a check that we
 * do indeed have a dynamically allocated array.
 */
typedef struct {
    void*   data;         /* Pointer to the contiguous elements  */
    size_t  size;         /* How big is each element?            */
    long    rank;         /* number of dimensions                */
    long    magic;        /* magic_mark                          */
    size_t* shape;        /* What are those dimensions?          */
} header_t;

/* 
 * Define the magic mark to be embedded in the header_t
 */
#define magic_mark       0x1972

/* 
 * Define an alignment policy, such that the headers and the actual
 * data are a multiple of mem_align_bytes apart.
 */
#define mem_align_x      1
#define mem_align_bytes  (mem_align_x*sizeof(char*))
#define header_size      (((sizeof(header_t)+mem_align_bytes-1) \
                          /mem_align_bytes)*mem_align_bytes)
#define header_ptr_size  (header_size/sizeof(char*))


/*
 * INTERNAL ROUTINES
 *
 * (start with 'da_' and are static)                                     
 *
 */

/*
 * Internal function to get the hidden header given the
 * pointer-to-pointer array
 */
static 
header_t* da_get_header_address(const void* array)
{    
    return array==NULL?0:((header_t*)((char*)array - header_size));
}

/*
 * Internal function to check that a given header has the correct magic mark
 */
static 
int da_is_header(const header_t* hdr)
{
    return (hdr != NULL) && ( hdr->magic == magic_mark ); 
}

/*
 * Internal function to prepend the dynamic pointer with a header 
 */
static 
void da_create_header( void*    array,
                       void*    data,
                       size_t   size,
                       size_t   rank,
                       size_t*  shape )
{  
    header_t* hdr = da_get_header_address(array);
    hdr->data  = data;
    hdr->size  = size;
    hdr->rank  = rank;
    hdr->magic = magic_mark;
    hdr->shape = shape;
}

/*
 * Internal function to create the pointer-to-pointer structure for any rank 
 */
static 
void* da_create_array(void* data, size_t size, size_t rank, size_t*  shape)
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
 * Internal function to release the memory allocated by da_create_array 
 */
static 
void* da_destroy_array(void* ptr)
{
    if (ptr != NULL)
        free((char*)ptr - header_size);
}

/*
 * Internal function to create a dimension array from a va_list
 */
static 
size_t* da_create_shape(size_t rank, va_list arglist)
{
    size_t* shape = malloc(sizeof(size_t)*rank);    
    if (shape != NULL) {
        int i;
        for (i = 0; i < rank; i++) 
            shape[i] = va_arg(arglist, size_t);
    }
    return shape;
}

/*
 * Internal function to copy a dimension array
 */
static 
size_t* da_copy_shape(size_t rank, const size_t* from)
{
    size_t* shape = NULL;
    if (from != NULL) {
        shape = malloc(sizeof(size_t)*rank);    
        if (shape != NULL) {
            int i;
            for (i = 0; i < rank; i++) { 
                shape[i] = from[i];
            }
        }
    }
    return shape;
}

/*
 * Internal function to release the memory allocated by da_create_shape
 */
static 
void* da_destroy_shape(size_t* ptr)
{
    free(ptr);
}

/*
 * Internal function to determine total number of elements in a shape
 */
static 
size_t da_fullsize_shape(size_t rank, size_t* ptr)
{
    size_t fullsize, i;
    if (ptr == NULL) 
        return 0;
    fullsize = 1;
    for (i=0; i<rank; i++)
        fullsize *= ptr[i];
    return fullsize;
}

/*
 * Internal function to allocate uninitialized memory for data with
 * any required extra space for bookkeeping.
 */
static 
void* da_create_data(size_t nmemb, size_t size)
{
    char* data = malloc(nmemb*size + header_size);
    if (data != NULL)
        data += header_size;
    return (void*)data;
}

/*
 * Internal function to allocate zero-initialized memory for data with
 * any required extra space for bookkeeping.
 */
static 
void* da_create_clear_data(size_t nmemb, size_t size)
{
    size_t chunks = (nmemb*size+header_size+mem_align_bytes-1)
                    /mem_align_bytes;
    char* data = calloc(chunks, mem_align_bytes);
    if (data != NULL)
        data += header_size;
    return (void*)data;
}

/*
 * Internal function to reallocate memory for data with
 * any required extra space for bookkeeping, keeping old data values.
 */
static 
void* da_recreate_data(void* data, size_t nmemb, size_t size)
{
    if (data != NULL) {
        char* newdata = realloc((char*)data - header_size, 
                                nmemb*size + header_size);
        if (newdata != NULL)
            newdata += header_size;
        return (void*)newdata;
    } else
        return da_create_data(nmemb,size);
}

/*
 * Internal function to release the memory allocated by
 * da_create_data, da_recreate_data, or da_ccreate_data
 */
static 
void* da_destroy_data(void* data)
{
    if (data!=NULL)
        free((char*)data - header_size);
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
    void*    result;
    size_t*  shape;
    va_list  arglist;
    va_start(arglist, rank);
    shape = da_create_shape(rank, arglist);
    va_end(arglist);
    result = samalloc(size, rank, shape); /* calls non-variadic function */
    da_destroy_shape(shape);
    return result;
}
/* Non-variadic version */
void* samalloc(size_t size, size_t rank, const size_t* shape)
{
    size_t*  shapecopy;
    void*    array;
    void*    data;
    size_t   i, total_elements;

    if (shape == NULL) 
        return NULL;

    shapecopy = da_copy_shape(rank, shape);  
    if (shapecopy == NULL)
       return NULL;

    total_elements = da_fullsize_shape(rank, shapecopy);

    data = da_create_data(total_elements, size);
    if (data == NULL) {
        da_destroy_shape(shapecopy);
        return NULL;
    }

    array = da_create_array(data, size, rank, shapecopy);
    if (array == NULL) {
        da_destroy_shape(shapecopy);
        da_destroy_data(data);
    } else 
        da_create_header(array, data, size, rank, shapecopy);

    return array;
}

/*
 *  The 'acalloc' function has the same functionality as amalloc, but
 *  also initialized the array to all zeros (by calling 'calloc').
 */
void* acalloc(size_t size, size_t rank, ...)
{
    void*    result;
    size_t*  shape;
    va_list  arglist;
    va_start(arglist, rank);
    shape = da_create_shape(rank, arglist);
    va_end(arglist);
    result = sacalloc(size, rank, shape); /* calls non-variadic function */
    da_destroy_shape(shape);
    return result;
}
/* Non-variadic version */
void* sacalloc(size_t size, size_t rank, const size_t* shape)
{
    size_t*  shapecopy;
    void*    array;
    void*    data;
    size_t   i, total_elements;
    
    if (shape == NULL) 
        return NULL;
    
    shapecopy = da_copy_shape(rank, shape);  
    if (shapecopy == NULL)
       return NULL;

    total_elements = da_fullsize_shape(rank, shapecopy);    

    data = da_create_clear_data(total_elements, size);
    if (data == NULL) {
        da_destroy_shape(shapecopy);    
        return NULL;
    }

    array = da_create_array(data, size, rank, shapecopy);
    if (array == NULL) {
        da_destroy_shape(shapecopy);
        da_destroy_data(data);
    } else
        da_create_header(array, data, size, rank, shapecopy);

    return array;
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
    size_t*  shape;
    va_list arglist;
    va_start(arglist, rank);
    shape = da_create_shape(rank, arglist);
    va_end(arglist);
    result = sarealloc(ptr, size, rank, shape);
    da_destroy_shape(shape);
    return result;
}
/* Non-variadic version */
void* sarealloc(void* ptr, size_t size, size_t rank, const size_t* shape)
{
    void*      array;
    void*      olddata;
    void*      data;
    size_t     i, total_elements, oldrank;
    size_t*    oldshape;
    size_t*    shapecopy;
    header_t*  hdr;
    
    if (shape == NULL) 
        return NULL;

    shapecopy = da_copy_shape(rank, shape);

    hdr = da_get_header_address(ptr);
    olddata  = hdr->data;
    oldshape = hdr->shape;
    oldrank  = hdr->rank;

    total_elements = da_fullsize_shape(rank, shapecopy);    

    data = da_recreate_data(olddata, total_elements, size);
    if (data == NULL) {
        da_destroy_shape(shapecopy);
        return NULL;
    }
    array = da_create_array(data, size, rank, shapecopy);
    if (array == NULL) {
        da_destroy_shape(shapecopy);
        da_destroy_data(data);
        return NULL;
    } else {
        da_create_header(array, data, size, rank, shapecopy);
        da_destroy_shape(oldshape);
        if (oldrank > 1) 
            da_destroy_array(ptr);
        return array;
    }
}

/*
 *  The 'afree' function frees up all the memory allocated for the
 *  multi-dimensional array associates with the pointer 'ptr'.
 */
void afree(void* ptr)
{
    if (ptr != NULL) {
        header_t* hdr = da_get_header_address(ptr);
        if (da_is_header(hdr)) {
            da_destroy_shape(hdr->shape);
            if (hdr->data && hdr->rank > 1) 
                da_destroy_data(hdr->data);
            da_destroy_array(ptr);
        }
    }
}

/*
 * Function to get the extent in any given dimension. 
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.
 */
size_t asize(const void* ptr, size_t dim)
{
    header_t* hdr = da_get_header_address(ptr);
    if (da_is_header(hdr)) {
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
    const header_t* hdr = da_get_header_address(ptr);
    if (da_is_header(hdr))
        return hdr->data;
    else
        return NULL;
}
const void* acdata(const void* ptr)
{
    const header_t* hdr = da_get_header_address(ptr);
    if (da_is_header(hdr))
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
size_t arank(const void* ptr)
{
    const header_t* hdr = da_get_header_address(ptr);
    if (da_is_header(hdr))
        return hdr->rank;
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
const size_t* ashape(const void* ptr)
{
    header_t* hdr = da_get_header_address(ptr);
    if (da_is_header(hdr))
        return hdr->shape;
    else
        return 0;
}

/*
 * Function to get the total number of elements in the multi-dimensional  
 * array. This works because an internal header containing the information about
 * about the multi-dimensional structure is associated with each dynamicaly
 * allocated multi-dimensional array. Returns 0 if no dynamic array
 * is associated with 'ptr'.
 */
size_t afullsize(const void* ptr)
{
    header_t* hdr = da_get_header_address(ptr);
    if (da_is_header(hdr))
        return da_fullsize_shape(hdr->rank, hdr->shape);
    else
        return 0;
}

/* 
 * Function to check if 'ptr's is an array allocated with amalloc,
 * acalloc, or arealloc.  This works because an internal header
 * containing the information about the multi-dimensional structure is
 * associated with each dynamicaly allocated multi-dimensional array.
 */
int aknown(const void* ptr)
{
    header_t* hdr = da_get_header_address(ptr);
    return da_is_header(hdr);
}

/* end of file amalloc.c */
