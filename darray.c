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
   struct. A special magic_mark is added to the number_extents field,
   as a check that we do indeed have a darray struct. */
typedef struct {
  void*   data;            // data, same as in darray
  size_t  element_size;    // element_size, same as in darray
  size_t  number_extents;  // same as in darray, but with magic_mark added
  size_t* extent;          // same as in darray
} header;

/* The magic mark to be embedded in number_extents */
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
  hdr->number_extents &= magic_unmask;
  hdr->number_extents |= magic_mark;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* Check that a given header has the correct magic stamp */
static int da_is_header(header* hdr)
{
  return (hdr->number_extents & magic_mask) == magic_mark;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* Create a darray structure and prepend its array with a header */
static darray da_create( void*   array,
                         void*   data,
                         size_t  element_size,
                         size_t  number_extents,
                         size_t* extent )
{  
  header* hdr = da_get_header(array);
  hdr->data           = data;
  hdr->element_size   = element_size;
  hdr->number_extents = number_extents;
  hdr->extent         = extent;
  da_mark_header(hdr);
  return (darray){array, data, element_size, number_extents, extent};
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* Create the pointer-to-pointer structure for any rank */
static void* da_pointer_structure( void*   data,
                                   size_t  element_size,
                                   size_t  number_extents,
                                   size_t* extent )
{
  if (number_extents <= 1) {

    return data;

  } else {

    size_t  i, j, ntot;
    size_t  nalloc;    
    char**  palloc;
    char**  result;
    char*** ptr;

    nalloc = 0;
    for (i = number_extents-1; i--; )
      nalloc = extent[i]*(1+nalloc);

    palloc = (char**)calloc(nalloc + header_ptr_size, sizeof(char*));
    if (palloc == NULL)
      return NULL;
    palloc += header_ptr_size;

    ntot = 1;   
    ptr = &result;
    for (i = 0; i < number_extents - 1; i++) {    
      for (j = 0; j < ntot; j++)
        ptr[j] = palloc + j*extent[i];      
      ptr = (char***)(*ptr);
      ntot *= extent[i];
      palloc += ntot;
    }
    for (j = 0; j < ntot; j++)
      ptr[j] = (char**)((char*)data 
                        + element_size*j*extent[number_extents-1]);
    return (void*)result;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

static darray vdmalloc(size_t size, size_t nextents, va_list arglist)
{
  void*    array;
  void*    data;
  size_t*  extent;
  size_t   i, total_elements;

  extent = malloc(sizeof(size_t)*nextents);
  if (extent == NULL) 
    return DNULL;

  for (i = 0; i < nextents; i++) 
    extent[i] = va_arg(arglist, size_t);

  total_elements = 1;
  for (i = 0; i < nextents; i++) 
    total_elements *= extent[i];

  data = malloc(total_elements*size + header_size);
  if (data == NULL) {
    free(extent);
    return DNULL;
  }
  data = (char*)data + header_size;

  array = da_pointer_structure(data, size, nextents, extent);
  if (array == NULL) {
    free(extent);
    free((char*)data - header_size);
    return DNULL;
  } else 
    return da_create(array, data, size, nextents, extent);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

static darray vdcalloc(size_t size, size_t nextents, va_list arglist)
{
  void*    array;
  void*    data;
  size_t*  extent;
  size_t   i, total_elements;

  extent = malloc(sizeof(size_t)*nextents);
  if (extent == NULL) 
    return DNULL;

  for (i = 0; i < nextents; i++) 
    extent[i] = va_arg(arglist, size_t);

  total_elements = 1;
  for (i = 0; i < nextents; i++) 
    total_elements *= extent[i];

  size_t chunks = (total_elements*size+header_size+mem_align_bytes-1)/mem_align_bytes;
  data = calloc(chunks, mem_align_bytes);
  if (data == NULL) {
    free(extent);
    return DNULL;
  }
  data = (char*)data + header_size;

  array = da_pointer_structure(data, size, nextents, extent);
  if (array == NULL) {
    free(extent);
    free((char*)data - header_size);
    return DNULL;
  } else
    return da_create(array, data, size, nextents, extent);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

static darray vdrealloc(darray darr, size_t size, size_t nextents, va_list arglist)
{
  void*    array;
  void*    data;
  size_t*  extent;
  size_t   i, total_elements;

  free(darr.extent);
  free((char*)darr.array + header_size);

  extent = malloc(sizeof(size_t)*nextents);
  if (extent == NULL) 
    return DNULL;

  for (i = 0; i < nextents; i++) 
    extent[i] = va_arg(arglist, size_t);

  total_elements = 1;
  for (i = 0; i < nextents; i++) 
    total_elements *= extent[i];

  data = realloc((char*)darr.data - header_size, 
                 total_elements*size + header_size);
  if (data == NULL) {
    free(extent);
    return DNULL;
  }
  data = (char*)data + header_size;

  array = da_pointer_structure(data, size, nextents, extent);
  if (array == NULL) {
    free(extent);
    free((char*)data - header_size);
    return DNULL;
  } else
    return da_create(array, data, size, nextents, extent);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 


/*                            *\ 
   IMPLEMENTATION OF THE API
\*                            */


darray dmalloc(size_t size, size_t nextents, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, nextents);
  #ifdef __TINYC__
  // tcc stdarg is off by one element in struct returning functions
  va_arg(arglist, size_t);
  #endif
  result = vdmalloc(size, nextents, arglist);
  va_end(arglist);
  return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

darray dcalloc(size_t size, size_t nextents, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, nextents);
  #ifdef __TINYC__
  // tcc stdarg is off by one element in struct returning functions
  va_arg(arglist, size_t);
  #endif
  result = vdcalloc(size, nextents, arglist);
  va_end(arglist);
  return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

darray drealloc(darray darr, size_t size, size_t nextents, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, nextents);
  #ifdef __TINYC__
  // tcc stdarg is off by one element in struct returning functions
  va_arg(arglist, size_t);
  #endif
  result = vdrealloc(darr, size, nextents, arglist);
  va_end(arglist);
  return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void dfree(darray darr)
{  
  free(darr.extent);
  free((char*)darr.data - header_size);
  if (darr.number_extents > 1) 
    free((char*)darr.array - header_size);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

size_t dextentof(darray darr, size_t dim)
{
  return (dim < darr.number_extents) ? darr.extent[dim] : 0;
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
  for (i = 0; i < a.number_extents-1; i++) {
    n = va_arg(arglist, size_t);
    start = (char**)(start[n]);
  }
  n = va_arg(arglist, size_t);
  va_end(arglist);

  y = (void*)((char*)start + n*a.element_size);

  memcpy(x, y, a.element_size);  
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
  for (i = 0; i < a.number_extents-1; i++) {
    n = va_arg(arglist, size_t);
    start = (char**)(start[n]);
  }
  n = va_arg(arglist, size_t);
  va_end(arglist);

  y = (void*)((char*)start + n*a.element_size);

  memcpy(y, x, a.element_size); 
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
        hdr->element_size, 
        hdr->number_extents & magic_unmask, 
        hdr->extent
      };
  } else {
    return DNULL;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
int dnotnull(darray darr)
{
  return darr.array != NULL || darr.data != NULL || darr.extent != NULL
    || darr.number_extents != 0 || darr.element_size != 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void* amalloc(size_t size, size_t nextents, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, nextents);
  result = vdmalloc(size, nextents, arglist);
  va_end(arglist);
  if ( dnotnull(result) )
    return result.array;
  else
    return NULL;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void* acalloc(size_t size, size_t nextents, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, nextents);
  result = vdcalloc(size, nextents, arglist);
  va_end(arglist);
  if ( dnotnull(result) )
    return result.array;
  else
    return NULL;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

void* arealloc(void* arr, size_t size, size_t nextents, ...)
{
  darray result;
  va_list arglist;
  va_start(arglist, nextents);
  result = vdrealloc(atod(arr), size, nextents, arglist);
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

int isdarray(void* arr)
{
  header* hdr = da_get_header(arr);
  return da_is_header(hdr);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
