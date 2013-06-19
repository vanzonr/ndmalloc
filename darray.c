/*
 * darray.h
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

/* Define a partial darray struct, just enough to be able to convert
   say a double** to a darray.  This header will be before the
   addresses hold in the data and array pointers in the darray
   struct. A special magic_mark is added to the rank field,
   as a check that we do indeed have a darray struct. */
typedef struct {
  void*   data;    // same as in darray
  size_t  size;    // same as in darray
  size_t  rank;    // same as in darray, but with magic_mark added
  size_t* shape;   // same as in darray
} header;

/* The magic mark to be embedded in rank */
#define magic_mark       0x19720000
#define magic_mask       0xffff0000
#define magic_unmask     0x0000ffff

/* Define an alignment policy, such that the headers and the actual
   data are a multiple of mem_align_bytes apart. */
#define mem_align_x      4
#define mem_align_bytes  (mem_align_x*sizeof(char*))
#define header_size      (((sizeof(header)+mem_align_bytes-1) \
                          /mem_align_bytes)*mem_align_bytes)
#define header_ptr_size  (header_size/sizeof(char*))

/*                                    *\  
   INTERNAL ROUTINES START WITH 'da_' 
\*                                    */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* Get the hidden header given the pointer-to-pointer array */
static header* da_get_header(void* array)
{
  return (header*)((char*)array - header_size);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* Mark a given header with the correct magic stamp */
static void da_mark_header(header* hdr)
{
  hdr->rank &= magic_unmask;
  hdr->rank |= magic_mark;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* Check that a given header has the correct magic stamp */
static int da_is_header(header* hdr)
{
  return (hdr->rank & magic_mask) == magic_mark;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* Create a darray structure and prepend its array with a header */
static darray da_create( void*   array,
                         void*   data,
                         size_t  size,
                         size_t  rank,
                         size_t* shape )
{  
  header* hdr = da_get_header(array);
  hdr->data  = data;
  hdr->size  = size;
  hdr->rank  = rank;
  hdr->shape = shape;
  da_mark_header(hdr);
  return (darray){array, data, size, rank, shape};
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* Create the pointer-to-pointer structure for any rank */
static void* da_pointer_structure( void*   data,
                                   size_t  size,
                                   size_t  rank,
                                   size_t* shape )
{
  if (rank <= 1) {

    return data;

  } else {

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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

static darray da_vdmalloc(size_t size, size_t rank, va_list arglist)
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

  array = da_pointer_structure(data, size, rank, shape);
  if (array == NULL) {
    free(shape);
    free((char*)data - header_size);
    return DNULL;
  } else 
    return da_create(array, data, size, rank, shape);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

static darray da_vdcalloc(size_t size, size_t rank, va_list arglist)
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

  size_t chunks = (total_elements*size+header_size+mem_align_bytes-1)/mem_align_bytes;
  data = calloc(chunks, mem_align_bytes);
  if (data == NULL) {
    free(shape);
    return DNULL;
  }
  data = (char*)data + header_size;

  array = da_pointer_structure(data, size, rank, shape);
  if (array == NULL) {
    free(shape);
    free((char*)data - header_size);
    return DNULL;
  } else
    return da_create(array, data, size, rank, shape);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

static darray da_vdrealloc(darray darr, size_t size, size_t rank, va_list arglist)
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

  array = da_pointer_structure(data, size, rank, shape);
  if (array == NULL) {
    free(shape);
    free((char*)data - header_size);
    return DNULL;
  } else
    return da_create(array, data, size, rank, shape);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 


/*                            *\ 
   IMPLEMENTATION OF THE API
\*                            */


darray dmalloc(size_t size, size_t rank, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, rank);
  #ifdef __TINYC__
  // tcc stdarg is off by one element in struct returning functions
  va_arg(arglist, size_t);
  #endif
  result = da_vdmalloc(size, rank, arglist);
  va_end(arglist);
  return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

darray dcalloc(size_t size, size_t rank, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, rank);
  #ifdef __TINYC__
  // tcc stdarg is off by one element in struct returning functions
  va_arg(arglist, size_t);
  #endif
  result = da_vdcalloc(size, rank, arglist);
  va_end(arglist);
  return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

darray drealloc(darray darr, size_t size, size_t rank, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, rank);
  #ifdef __TINYC__
  // tcc stdarg is off by one element in struct returning functions
  va_arg(arglist, size_t);
  #endif
  result = da_vdrealloc(darr, size, rank, arglist);
  va_end(arglist);
  return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void dfree(darray darr)
{  
  free(darr.shape);
  free((char*)darr.data - header_size);
  if (darr.rank > 1) 
    free((char*)darr.array - header_size);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

size_t dextentof(darray darr, size_t dim)
{
  return (dim < darr.rank) ? darr.shape[dim] : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void dget(void* x, darray a, ...)
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void dset(void* x, darray a, ...)
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void* dtoa(darray darr)
{
  return darr.array;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

darray atod(void* arr)
{
  header* hdr = da_get_header(arr);
  if (da_is_header(hdr)) {
    return (darray) 
      {
        arr, 
        hdr->data, 
        hdr->size, 
        hdr->rank & magic_unmask, 
        hdr->shape
      };
  } else {
    return DNULL;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
int dnotnull(darray darr)
{
  return darr.array != NULL || darr.data != NULL || darr.shape != NULL
    || darr.rank != 0 || darr.size != 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void* amalloc(size_t size, size_t rank, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, rank);
  result = da_vdmalloc(size, rank, arglist);
  va_end(arglist);
  if ( dnotnull(result) )
    return result.array;
  else
    return NULL;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void* acalloc(size_t size, size_t rank, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, rank);
  result = da_vdcalloc(size, rank, arglist);
  va_end(arglist);
  if ( dnotnull(result) )
    return result.array;
  else
    return NULL;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void* arealloc(void* arr, size_t size, size_t rank, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, rank);
  result = da_vdrealloc(atod(arr), size, rank, arglist);
  va_end(arglist);
  
  if ( dnotnull(result) )
    return result.array;
  else
    return NULL;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void afree(void* arr)
{
  darray darr = atod(arr);
  if ( dnotnull(darr) )
    dfree(darr);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

size_t extentof(void* arr, size_t dim)
{
  darray darr = atod(arr);
  if ( dnotnull(darr) )
    return dextentof(darr, dim);
  else
    return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

int aisd(void* arr)
{
  header* hdr = da_get_header(arr);
  return da_is_header(hdr);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
