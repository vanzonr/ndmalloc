/*
 * ndmalloc.c
 *
 * Implementation of the ndmalloc library, which can make arbitrary
 * multi-dimensional c arrays ('nd arrays'). 
 *
 * Copyright (c) 2013 Ramses van Zon
 */

#include <stdlib.h>
#include <stdarg.h>
#include "areg.ic"

/* Note: in areg.ic, AREG_INT should set both a_index_t and areg_clue_t. */

/***************************************************************************/

struct header {

 /* Holds information about the dynamically allocated
    array.  This header will be placed before the addresses holding the
    data and array pointers. A special magic_mark is added, as a first
    check that we do indeed have a dynamically allocated array, and to
    indicate whether this array is only a view on another array. */

    areg_clue_t  clue;         /* clue for areg                  */
    short        rank;         /* number of dimensions           */
    short        magic;        /* magic_mark                     */
    size_t*      shape;        /* What are those dimensions?     */
};

/* Define the magic mark to be embedded in the struct header.  These
   constant definitions presume at least 32 bits in an int, but will
   still work with less bits. */

static short magic_mark      = 0x1972; /* in headers of allocated arrays */
static short view_magic_mark = 0x1973; /* in headers of views on arrays  */

/* Define an alignment policy, such that the headers and the actual
   data are a multiple of mem_align_bytes apart. Note that the only
   true requirement for ndmalloc is that the distance between header
   and actual data is a multiple of the size of a pointer. */

#define mem_align_x      1
#define mem_align_bytes  (mem_align_x*sizeof(char*))
#define header_size      (((sizeof(struct header)+mem_align_bytes-1) \
                          /mem_align_bytes)*mem_align_bytes)
#define header_ptr_size  (header_size/sizeof(char*))

/***************************************************************************/

/*
 * INTERNAL ROUTINES
 *
 * (start with 'da_' and are static inline)
 */

/***************************************************************************/
 
static 
struct header* da_get_header_address(const void* array)
{    
 /* Get the hidden header given the pointer-to-pointer array */

    return array==NULL?0:((struct header*)((char*)array - header_size));
}

/***************************************************************************/
 
static 
void da_create_header( void*       array,
                       short       rank,
                       size_t*     shape,
                       short       mark,
                       areg_clue_t clue )
{  
 /* Prepend a pointer with a header */

    struct header* hdr;

    hdr = da_get_header_address(array);
    hdr->clue  = clue;
    hdr->rank  = rank;
    hdr->magic = mark;
    hdr->shape = shape;
}

/***************************************************************************/
 
static 
void* da_create_array( void*        data, 
                       size_t       size, 
                       short        rank, 
                       size_t*      shape, 
                       areg_clue_t* clue)
{
 /* Create the pointer-to-pointer structure for any rank */

    short   i;
    size_t  j, ntot;
    size_t  nalloc;    
    char**  palloc;
    char**  result;
    char*** ptr;

    if (rank <= 1) {
        
        if (areg_add(data, clue) == AREG_SUCCESS)
            return data;
        else
            return NULL;
       
    } else {
                
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

/***************************************************************************/
 
static 
int da_destroy_array(void* ptr, areg_clue_t clue)
{
 /* Release the memory allocated by da_create_array */

    if (ptr != NULL) {
        free((char*)ptr - header_size);
        return areg_remove(ptr, clue);
    } else 
        return AREG_SUCCESS;
}

/***************************************************************************/
 
static 
size_t* da_create_shape(short rank, va_list arglist)
{
 /* Create a dimension array from a va_list */

    size_t* shape;
    short   i;
    size_t  fullsize;
    size_t  n;

    if (rank > 1 ) {
        shape = malloc(sizeof(size_t)*(rank+1));
        if (shape != NULL) {
            fullsize = 1;
            for (i = 0; i < rank; i++) {
                n = va_arg(arglist, size_t);
                shape[i] = n;
                fullsize *= n;
            }
            shape[rank]=fullsize;
        }
    } else {
        shape = malloc(sizeof(size_t));
        if (shape != NULL)
            shape[0] = va_arg(arglist, size_t);
    }

    return shape;
}

/***************************************************************************/

static 
size_t* da_copy_shape(short rank, const size_t* from)
{
 /* Copy a dimension array */

    size_t* shape;

    shape = NULL;
    if (from != NULL) {
        if (rank>1) {
            shape = malloc(sizeof(size_t)*(rank+1));
            if (shape != NULL) {
                short i;
                for (i = 0; i < rank+1; i++) { 
                    shape[i] = from[i];
                }
            }
        } else {
            shape = malloc(sizeof(size_t));    
            if (shape != NULL) 
                shape[0] = from[0];
        }
    }

    return shape;
}

/***************************************************************************/

static 
void da_destroy_shape(size_t* ptr)
{
 /* Release the memory allocated by da_create_shape */

    free(ptr);
}

/***************************************************************************/
 
static 
size_t da_fullsize_shape(short rank, size_t* ptr)
{
 /* Determine total number of elements in a shape */

    if (rank > 1)
        return ptr[rank];
    else
        return ptr[0];
}

/***************************************************************************/

static 
void* da_create_data(size_t nmemb, size_t size)
{
 /* Allocate uninitialized memory for data. */

    char* data;

    data = malloc(nmemb*size + header_size);
    if (data != NULL)
        data += header_size;

    return (void*)data;
}

/***************************************************************************/
 
static 
void* da_create_clear_data(size_t nmemb, size_t size)
{
 /* Allocate zero-initialized memory for data with any required extra
    space for bookkeeping. */

    size_t chunks;
    char*  data;

    chunks = (nmemb*size+header_size+mem_align_bytes-1)/mem_align_bytes;
    data = calloc(chunks, mem_align_bytes);
    if (data != NULL)
        data += header_size;

    return (void*)data;
}

/***************************************************************************/
 
static 
void* da_recreate_data(void* data, size_t nmemb, size_t size)
{
 /* Reallocate memory for data with any required
    extra space for bookkeeping, keeping old data values. */

    if (data != NULL) {
        char* newdata = realloc((char*)data - header_size, 
                                nmemb*size + header_size);
        if (newdata != NULL)
            newdata += header_size;
        return (void*)newdata;
    } else
        return da_create_data(nmemb,size);
}

/***************************************************************************/
 
static 
void da_destroy_data(void* data)
{
 /* Release the memory allocated by da_create_data, da_recreate_data,
    or da_ccreate_data */

    if (data!=NULL)
        free((char*)data - header_size);
}

/***************************************************************************/
 
static 
void* da_get_data(void* ptr, short rank)
{
 /* Get the pointer to the data for an ndmalloc array. */

    void** result;
    short  i;

    result = ptr;
    for (i = 0; i < rank-1; i++) 
        result = (void**)(*result);

    return (void*)result;
}

/***************************************************************************/

static 
const void* da_get_cdata(const void* ptr, short rank)
{
 /* Const version of da_get_data ('const' really is contagious). */

    void const*const* result;
    short i;
    
    result = ptr;
    for (i = 0; i < rank-1; i++) 
        result = (void const*const*)(*result);

    return (const void*)result;
}


/*
 * IMPLEMENTATION OF THE INTERFACE
 */

/***************************************************************************/

void* sndmalloc(size_t size, short rank, const size_t* shape)
{
 /* Creates a dynamically allocated multi- dimensional array of
    dimensions n[0] x n[1] ... x n['rank'-1], with elements of 'size'
    bytes.  The dimensions are to be given as the variable-length
    arguments.  The function allocates 'size'*n[0]*n[1]*..n['rank'-1]
    bytes for the data, plus another n[0]*n[1]*...n[rank-2]
    *sizeof(void*) bytes for the pointer-to-pointer structure that is
    common for c-style arrays.  It also allocates internal buffers of
    moderate size.  The pointer-to-pointer structure assumes that all
    pointers are the same size.  The return value can be cast to a
    TYPE* for an array of rank 1, TYPE** for rank 2, T*** for rank 3,
    etc.  .This casted pointer can then be used in the same way a
    c-style array is used, i.e., with repeated square bracket
    indexing.  If the memory allocation fails, a NULL pointer is
    returned.  The return value (or its casted version) can be used in
    calls to 'ndrealloc', 'ndfree', 'ndsize', 'nddata', 'ndrank',
    'ndshape', 'ndisknown'.  This works because an internal header
    containing the information about the multi-dimensional structure
    is associated with each dynamicaly allocated multi-dimensional
    array. */

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
    } else {
        da_create_header(array, rank, shapecopy, magic_mark, clue);
        if (rank > 1) {
            areg_add(data, &clue);
            da_create_header(data, 1, shapecopy+rank, view_magic_mark, clue);
        }
    }

    return array;
}

/***************************************************************************/

void* ndmalloc(size_t size, short rank, ...)
{
 /* Variadic version of sndmalloc */

    void*    result;
    size_t*  shape;
    va_list  arglist;

    va_start(arglist, rank);
    shape = da_create_shape(rank, arglist);
    va_end(arglist);
    result = sndmalloc(size, rank, shape); /* calls non-variadic function */
    da_destroy_shape(shape);

    return result;
}

/***************************************************************************/

void* sndcalloc(size_t size, short rank, const size_t* shape)
{
 /* Same functionality as ndmalloc, but also initialized the array to
    all zeros by calling 'calloc'. */

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
    } else {
        da_create_header(array, rank, shapecopy, magic_mark, clue);
        if (rank > 1) {
            areg_add(data, &clue);
            da_create_header(data, 1, shapecopy+rank, view_magic_mark, clue);
        }
    }

    return array;
}
/***************************************************************************/

void* ndcalloc(size_t size, short rank, ...)
{
 /* Variadic version of sndcalloc */

    void*    result;
    size_t*  shape;
    va_list  arglist;

    va_start(arglist, rank);
    shape = da_create_shape(rank, arglist);
    va_end(arglist);
    result = sndcalloc(size, rank, shape); /* calls non-variadic function */
    da_destroy_shape(shape);

    return result;
}

/***************************************************************************/

int ndisknown(const void* ptr)
{
 /* Check if 'ptr's is an array allocated with (s)ndmalloc,
    (s)ndcalloc, (s)ndrealloc, or (s)ndview. */

    struct header* hdr;

    hdr = da_get_header_address(ptr);

    return hdr != NULL 
        && (hdr->magic | 1) == (magic_mark | 1)
        && areg_lookup(ptr, &(hdr->clue)) == AREG_SUCCESS;
}

/***************************************************************************/

void* sndrealloc( void*          ptr, 
                  size_t         size, 
                  short          rank, 
                  const size_t*  shape )
{
 /* Changes the dimensions and/or the size of the multi-dimensional
    array 'ptr'.  The content of the array will be unchanged in the
    range from the start of the region up to the minimum of the old
    and new sizes.  If the change in dimensions has changed the shape,
    the elements get reassigned indices according to the row-major
    ordering.  If the re-allocation is succesful, the new pointer is
    returned and the old one is invalid.  If the function fails, NULL
    is returned.  Known bug: the original 'ptr' is still deallocated
    when 'sndrealloc' fails. */

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

    /* can only reshape ndmalloc arrays, not pointer, not views */
    if (hdr == NULL || ! ndisknown(ptr) || (hdr->magic&1) == 1 )
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
        /* areg_remove(data); areg_add(date); TO DO*/
        da_create_header(array, rank, shapecopy, magic_mark, clue);
        da_destroy_shape(oldshape);
        if (oldrank > 1) 
            (void)da_destroy_array(ptr,oldclue);/*should check error status*/
            /* note: destroy array does an areg_remove of ptr */
        else
            areg_remove(ptr, oldclue);
        return array;
    }
}

/***************************************************************************/

void* ndrealloc(void* ptr, size_t size, short rank, ...)
{
 /* Variadic version of sndrealloc. */

    void* result;
    size_t* shape;
    va_list arglist;

    va_start(arglist, rank);
    shape = da_create_shape(rank, arglist);
    va_end(arglist);
    result = sndrealloc(ptr, size, rank, shape);
    da_destroy_shape(shape);

    return result;
}

/***************************************************************************/

void ndfree(void* ptr)
{
 /* Free up all the memory allocated for the nd array 'ptr'. */

    struct header*  hdr;
    void*           data;

    if (ndisknown(ptr)) {
        hdr = da_get_header_address(ptr);
        /* safe guard against freeing a 1d view, which can only be
           created with nddata and ndcdata and should not be passed to
           ndfree, as it is part of another nd array. */
        if ( hdr->rank ==1 && (hdr->magic & 1) == 1 )
            return;
        da_destroy_shape(hdr->shape);
        /* for rank==1 data and array are the same */
        /* views should not have their data freed */
        if ( (hdr->rank > 1) && ((hdr->magic & 1) == 0) ) {
            data = da_get_data(ptr, hdr->rank);
            areg_remove(data, da_get_header_address(data)->clue);
            da_destroy_data(data);
        }
        (void)da_destroy_array(ptr, hdr->clue);/* should check error status*/
    }
}

/***************************************************************************/

int ndisview(const void* ptr)
{
 /* Check if 'ptr' is an nd array created with (s)aview. */

    struct header* hdr;

    hdr = da_get_header_address(ptr);

    return hdr != NULL 
        && hdr->magic == view_magic_mark
        && areg_lookup(ptr, &(hdr->clue)) == AREG_SUCCESS;
}

/***************************************************************************/

size_t ndsize(const void* ptr, short dim)
{
 /* Get the extent of nd array 'ptr' in any given dimension 'dim'. */

    struct header* hdr;

    hdr = da_get_header_address(ptr);
    if (dim < hdr->rank)
        return hdr->shape[dim];
    else
        return 0;
}

/***************************************************************************/

void* nddata(void* ptr)
{
 /* Get the start of the data (useful for library calls).  Result is
    undefined if 'ptr' was not created with (s)ndmalloc, (s)ndcalloc,
    (s)ndrealloc, or (s)ndview. */

    const struct header* hdr;

    hdr = da_get_header_address(ptr);

    return da_get_data(ptr, hdr->rank);
}

/***************************************************************************/

const void* ndcdata(const void* ptr)
{
 /* Const version of nddata. */

    return da_get_cdata(ptr, da_get_header_address(ptr)->rank);
}

/***************************************************************************/

short ndrank(const void* ptr)
{
 /* Get the rank of the multi-dimensional array.  Result is undefined
    if 'ptr' was not created with (s)ndmalloc, (s)acalloc, (s)arealloc
    or (s)aview. */

    return da_get_header_address(ptr)->rank;
}

/***************************************************************************/

const size_t* ndshape(const void* ptr)
{
 /* Get the shape of the nd array 'ptr'.  The result is undefined if
    no nd array is associated with 'ptr'. */

    return da_get_header_address(ptr)->shape;
}

/***************************************************************************/

size_t ndfullsize(const void* ptr)
{
 /* Get the total number of elements in the nd array 'ptr'.  The
    result is undefined if no nd array is associated with 'ptr'. */

    struct header* hdr;

    hdr = da_get_header_address(ptr);

    return da_fullsize_shape(hdr->rank, hdr->shape);
}

/***************************************************************************/

void* sndview( void*          data, 
               size_t         size, 
               short          rank, 
               const size_t*  shape )
{
 /* Allocate a multi-dimensional view on existing data. */

    size_t*      shapecopy;
    void*        array;
    areg_clue_t  clue = AREG_NOCLUE;

    if (shape == NULL || data == NULL || rank <= 1) 
        return NULL;

    shapecopy = da_copy_shape(rank, shape);  
    if (shapecopy == NULL)
       return NULL;

    /* if data is known array, use its data: */
    if (ndisknown(data)) {
        /* check that there are enough elements */
        if (ndfullsize(data) < da_fullsize_shape(rank, shapecopy))
           return NULL;
        /* get the data, not the pointer-to-pointer */
        data = nddata(data);
    }

    array = da_create_array(data, size, rank, shapecopy, &clue);
    if (array == NULL) 
        da_destroy_shape(shapecopy);
    else 
        da_create_header(array, rank, shapecopy, view_magic_mark, clue);

    return array;
}
/***************************************************************************/

void* ndview(void* data, size_t size, short rank, ...)
{
 /* Variadic version of sndview */

    void*    result;
    size_t*  shape;
    va_list  arglist;

    va_start(arglist, rank);
    shape = da_create_shape(rank, arglist);
    va_end(arglist);
    result = sndview(data, size, rank, shape);
    da_destroy_shape(shape);

    return result;
}

/***************************************************************************/
/* end of file ndmalloc.c */
