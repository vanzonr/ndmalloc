/*
 * amalloc.c
 *
 * Implementation of the amalloc library, which can make arbitrary
 * multi-dimensional c arrays. 
 *
 * Copyright (c) 2013 Ramses van Zon
 */

#include <stdlib.h>
#include <stdarg.h>
#include <areg.ic>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* 
 * Define a header struct, with all the information about the
 * dynamically allocated array, except the pointer-to-pointer.  This
 * header will be placed before the addresses holdinng the data and
 * array pointers. A special magic_mark is added, as a check that we
 * do indeed have a dynamically allocated array, and to indicate if
 * this array is only a view on another array.
 */
struct header {
    areg_clue_t  clue;         /* clue for areg                  */
    short        rank;         /* number of dimensions           */
    short        magic;        /* magic_mark                     */
    size_t*      shape;        /* What are those dimensions?     */
};

/* 
 * Define the magic mark to be embedded in the struct header.  These
 * constant definitions presume at least 32 bits in an int, but will
 * still work with less bits.
 */
static short magic_mark      = 0x1972; /* in headers of allocated arrays */
static short view_magic_mark = 0x1973; /* in headers of views on arrays  */

/* 
 * Define an alignment policy, such that the headers and the actual
 * data are a multiple of mem_align_bytes apart.
 */
#define mem_align_x      1
#define mem_align_bytes  (mem_align_x*sizeof(char*))
#define header_size      (((sizeof(struct header)+mem_align_bytes-1) \
                          /mem_align_bytes)*mem_align_bytes)
#define header_ptr_size  (header_size/sizeof(char*))

/*
 * INTERNAL ROUTINES
 *
 * (start with 'da_' and are static inline)
 *
 */

/*
 * Internal function to get the hidden header given the
 * pointer-to-pointer array
 */
 
static struct header* da_get_header_address(const void* array)
{    
    return array==NULL?0:((struct header*)((char*)array - header_size));
}

/*
 * Internal functions to prepend the dynamic pointer with a header 
 */
 
static void da_create_header( void*       array,
                              void*       data,
                              size_t      size,
                              short       rank,
                              size_t*     shape,
                              short       mark,
                              areg_clue_t clue)
{  
    struct header* hdr = da_get_header_address(array);
    /* note that 'data' is not actually stored */
    /* note that 'size' is not actually stored */
    hdr->clue  = clue;
    hdr->rank  = rank;
    hdr->magic = mark;
    hdr->shape = shape;
}

/*
 * Internal function to create the pointer-to-pointer structure for any rank 
 */
 
static void* da_create_array(void* data, size_t size, short rank, size_t* shape, areg_clue_t* clue)
{
    if (rank <= 1) {
        
        if (areg_add(data, clue) == AREG_SUCCESS)
            return data;
        else
            return NULL;
       
    } else {
        
        short   i;
        size_t  j, ntot;
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
        (void)areg_add(result, clue); /* should check error status */
        return (void*)result;
    }
}

/*
 * Internal function to release the memory allocated by da_create_array 
 */
 
static void da_destroy_array(void* ptr, areg_clue_t clue)
{
    if (ptr != NULL) {
        free((char*)ptr - header_size);
        (void)areg_remove(ptr, clue); /* should check error status */
    }
}

/*
 * Internal function to create a dimension array from a va_list
 */
 
static size_t* da_create_shape(short rank, va_list arglist)
{
    size_t* shape = malloc(sizeof(size_t)*rank);    
    if (shape != NULL) {
        short i;
        for (i = 0; i < rank; i++) 
            shape[i] = va_arg(arglist, size_t);
    }
    return shape;
}

/*
 * Internal function to copy a dimension array
 */
 
static size_t* da_copy_shape(short rank, const size_t* from)
{
    size_t* shape = NULL;
    if (from != NULL) {
        shape = malloc(sizeof(size_t)*rank);    
        if (shape != NULL) {
            short i;
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
 
static void da_destroy_shape(size_t* ptr)
{
    free(ptr);
}

/*
 * Internal function to determine total number of elements in a shape
 */
 
static size_t da_fullsize_shape(short rank, size_t* ptr)
{
    size_t fullsize;
    short  i;
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
 
static void* da_create_data(size_t nmemb, size_t size)
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
 
static void* da_create_clear_data(size_t nmemb, size_t size)
{
    size_t chunks = (nmemb*size+header_size+mem_align_bytes-1)
                    /mem_align_bytes;
    char* data = calloc(chunks, mem_align_bytes);
    if (data != NULL)
        data += header_size;
    return (void*)data;
}

/*
 * Internal function to reallocate memory for data with any required
 * extra space for bookkeeping, keeping old data values.
 */
 
static void* da_recreate_data(void* data, size_t nmemb, size_t size)
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
 
static void da_destroy_data(void* data)
{
    if (data!=NULL)
        free((char*)data - header_size);
}

/*
 * Internal function to get the pointer to the data for an amalloc
 * array.
 */
 
static void* da_get_data(void* ptr, short rank)
{
    void** result = ptr;
    short  i;
    for (i = 0; i < rank-1; i++) 
        result = (void**)(*result);
    return (void*)result;

}

/* const version ('const' really is contagious). */

static const void* da_get_cdata(const void* ptr, short rank)
{
    void const*const* result = ptr;
    short i;
    for (i = 0; i < rank-1; i++) 
        result = (void const*const*)(*result);
    return (const void*)result;
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
void* samalloc(size_t size, short rank, const size_t* shape)
{
    size_t*       shapecopy;
    void*         array;
    void*         data;
    size_t        total_elements;
    areg_clue_t   clue = AREG_NOCLUE;

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

    array = da_create_array(data, size, rank, shapecopy, &clue);
    if (array == NULL) {
        da_destroy_shape(shapecopy);
        da_destroy_data(data);
    } else 
        da_create_header(array, data, size, rank, shapecopy, magic_mark, clue);

    return array;
}
/* Variadic version */
void* amalloc(size_t size, short rank, ...)
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

/*
 *  The 'acalloc' function has the same functionality as amalloc, but
 *  also initialized the array to all zeros (by calling 'calloc').
 */
void* sacalloc(size_t size, short rank, const size_t* shape)
{
    size_t*      shapecopy;
    void*        array;
    void*        data;
    size_t       total_elements;
    areg_clue_t  clue = AREG_NOCLUE;
    
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

    array = da_create_array(data, size, rank, shapecopy, &clue);
    if (array == NULL) {
        da_destroy_shape(shapecopy);
        da_destroy_data(data);
    } else
        da_create_header(array, data, size, rank, shapecopy, magic_mark, clue);

    return array;
}
/* Variadic version */
void* acalloc(size_t size, short rank, ...)
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

/* 
 * Function to check if 'ptr's is an array allocated with (s)amalloc,
 * (s)acalloc, (s)arealloc, or (s)aview.
 */
int aknown(const void* ptr)
{
    struct header* hdr = da_get_header_address(ptr);
    return hdr != NULL 
        && areg_lookup(ptr, &(hdr->clue)) == AREG_SUCCESS;
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
void* sarealloc(void* ptr, size_t size, short rank, const size_t* shape)
{
    void*           array;
    void*           olddata;
    void*           data;
    size_t          total_elements;
    short           oldrank;
    areg_clue_t     oldclue;
    size_t*         oldshape;
    size_t*         shapecopy;
    struct header*  hdr;
    areg_clue_t     clue = AREG_NOCLUE;
    
    if (shape == NULL) 
        return NULL;

    hdr = da_get_header_address(ptr);

    /* can only reshape amalloc arrays, not pointer, not views */
    if (hdr == NULL || ! aknown(ptr) || (hdr->magic&1) == 1 )
      return NULL;

    olddata  = da_get_data(ptr, rank);
    oldshape = hdr->shape;
    oldrank  = hdr->rank;
    oldclue  = hdr->clue;

    shapecopy = da_copy_shape(rank, shape);

    total_elements = da_fullsize_shape(rank, shapecopy);    

    data = da_recreate_data(olddata, total_elements, size);
    if (data == NULL) {
        da_destroy_shape(shapecopy);
        return NULL;
    }
    array = da_create_array(data, size, rank, shapecopy, &clue);
    if (array == NULL) {
        da_destroy_shape(shapecopy);
        da_destroy_data(data);
        return NULL;
    } else {
        da_create_header(array, data, size, rank, shapecopy, magic_mark, clue);
        da_destroy_shape(oldshape);
        if (oldrank > 1) 
            da_destroy_array(ptr, oldclue);
        else
            areg_remove(ptr, oldclue);
        return array;
    }
}
/* Variadic version */
void* arealloc(void* ptr, size_t size, short rank, ...)
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

/*
 *  The 'afree' function frees up all the memory allocated for the
 *  multi-dimensional array associates with the pointer 'ptr'.
 */
void afree(void* ptr)
{
    if (aknown(ptr)) {
        struct header* hdr = da_get_header_address(ptr);
        da_destroy_shape(hdr->shape);
        if ( (hdr->rank > 1) && ((hdr->magic & 1) == 0) ) {
            void* data = da_get_data(ptr, hdr->rank);
            da_destroy_data(data);
        }
        da_destroy_array(ptr, hdr->clue);
    }
}

/* 
 * Function to check if 'ptr' is an array created with (s)aview.
 */
int aisview(const void* ptr)
{
    struct header* hdr = da_get_header_address(ptr);
    return hdr != NULL 
        && areg_lookup(ptr, &(hdr->clue)) == AREG_SUCCESS 
        && (hdr->magic & 1) == 1 ;
}

/*
 * Function to get the extent in any given dimension. 
 * This works because an internal header containing the information
 * about the multi-dimensional structure is associated with each
 * dynamicaly allocated multi-dimensional array.
 */
size_t asize(const void* ptr, short dim)
{
    struct header* hdr = da_get_header_address(ptr);
    if (dim < hdr->rank)
        return hdr->shape[dim];
    else
        return 0;
}

/*
 * Function to get the start of the data (useful for library calls).
 * Returns NULL if 'ptr' was not created with (s)amalloc, (s)acalloc,
 * (s)arealloc, or (s)aview.
 */
void* adata(void* ptr)
{
    const struct header* hdr = da_get_header_address(ptr);
    return da_get_data(ptr, hdr->rank);
}

/* const version */
const void* acdata(const void* ptr)
{
    return da_get_cdata(ptr, da_get_header_address(ptr)->rank);
}

/*
 * Function to get the rank of the multi-dimensional array.  Returns 0
 * if 'ptr' was not created with (s)amalloc, (s)acalloc, (s)arealloc
 * or (s)aview.
 */
short arank(const void* ptr)
{
    return da_get_header_address(ptr)->rank;
}

/*
 * Function to get the shape of the multi-dimensional array.  Returns
 * NULL if no dynamic array is associated with 'ptr'.
 */
const size_t* ashape(const void* ptr)
{
    return da_get_header_address(ptr)->shape;
}

/*
 * Function to get the total number of elements in the
 * multi-dimensional array.  Returns 0 if no dynamic array is
 * associated with 'ptr'.
 */
size_t afullsize(const void* ptr)
{
    struct header* hdr = da_get_header_address(ptr);
    return da_fullsize_shape(hdr->rank, hdr->shape);
}

/*
 * Function 'aview' allocates a multi-dimensional view on existing data.
 */
void* saview(void* data, size_t size, short rank, const size_t* shape)
{
    size_t*      shapecopy;
    void*        array;
    areg_clue_t  clue = AREG_NOCLUE;

    if (shape == NULL || data == NULL || rank <= 1) 
        return NULL;

    shapecopy = da_copy_shape(rank, shape);  
    if (shapecopy == NULL)
       return NULL;

    /* if data is known: */
    if (aknown(data)) {
        /* check that there are enough elements */
        if (afullsize(data) < da_fullsize_shape(rank, shapecopy))
           return NULL;
        /* make sure we have the data, not the pointer-to-pointer */
        data = adata(data);
    }

    array = da_create_array(data, size, rank, shapecopy, &clue);
    if (array == NULL) 
        da_destroy_shape(shapecopy);
    else 
        da_create_header(array, data, size, rank, shapecopy, view_magic_mark, clue);

    return array;
}
/* Variadic version */
void* aview(void* data, size_t size, short rank, ...)
{
    void*    result;
    size_t*  shape;
    va_list  arglist;
    va_start(arglist, rank);
    shape = da_create_shape(rank, arglist);
    va_end(arglist);
    result = saview(data, size, rank, shape);
    da_destroy_shape(shape);
    return result;
}

/* end of file amalloc.c */
