/*****************************************************************/
/*                                                               */
/*  areg.h: register for pointers to be used with amalloc.       */
/*                                                               */
/*  (C) Copyright 2013 Ramses van Zon                            */
/*                                                               */
/*****************************************************************/

#ifndef AREG_RVZ
#define AREG_RVZ

#include <stdlib.h>

typedef const void*  ptr_t;          /* store constant pointers only */
typedef size_t       clue_t;         /* use clues for faster lookup  */

#define AREG_NOCLUE   ((clue_t)(0))  /* error/ignorance              */
#define AREG_SUCCESS  0              /* function call successful     */
#define AREG_FAILURE  1              /* an error occurred            */

/* Register a pointer.  Return a clue to where to find pointer in
   *clue.  Returns and error code, which is AREG_SUCCESS if the
   addition was successful, and AREG_FAILURE if it was not. */

int areg_add(const void* ptr, clue_t* clue);


/* Remove the pointer-value pair associated with 'ptr'.  Pass in a clue
   that was given by areg_add() for faster lookup, or '0'.  Returns AREG_SUCCESS if pair is
   removed and 1 if 'pointer' was not found. */

int areg_remove(const void* pointer, clue_t clue);


/* Checks if 'ptr' was registerd.  Pass in clue given by areg_add()
   for faster lookup. Returns AREG_SUCCESS if pointer is found,
   AREG_FAILURE if it is not. */

int areg_lookup(const void* pointer, clue_t clue);

#endif
