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

#include <stddef.h>

/* allocation: */
void*         amalloc  (size_t size, size_t rank, ...);
void*         acalloc  (size_t size, size_t rank, ...);
void*         arealloc (void*  ptr,  size_t size, size_t rank, ...);

/* deallocation: */
void          afree    (void*  ptr);

/* properties: */
size_t        asize    (void*  ptr,  size_t dim);
void*         adata    (void*  ptr);
size_t        arank    (void*  ptr);
const size_t* ashape   (void*  ptr);
int           aknown   (void*  ptr);

#endif

