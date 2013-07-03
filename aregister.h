/**********************************************************************/
/*                                                                    */
/*  aregister.h: register for pointers to be used with amalloc.       */
/*                                                                    */
/*  (C) Copyright 2013 Ramses van Zon                                 */
/*                                                                    */
/**********************************************************************/

#ifndef AREGISTERA_RVZ
#define AREGISTERA_RVZ

typedef size_t clue_t;                 /* use clues for faster lookup */

#define AREG_NOCLUE   ((clue_t)(-1))   /* error/ignorance */
#define AREG_SUCCESS  0                /* function call successful    */
#define AREG_FAILURE  1                /* an error occurred           */

/* Add a key-value entry.  Return a clue to where to find key. Returns
   and error code, which is 0 if the addition was successful. */

int areg_add(const void* key, void* val, clue_t* clue);


/* Remove the key-value pair associated with 'key'.  Pass in a clue
   that was given by areg_add() for faster lookup.  Returns 0 if pair is
   removed and 1 if 'key' was not found. */

int areg_remove(const void* key, clue_t clue);


/* Returns the value associated with 'key'.  Pass in clue given by
   areg_add() for faster lookup. Returns AREG_FAILURE if key is not
   found, and sets value to NULL. */

int areg_lookup(const void* key, clue_t clue, void** value);

#endif
