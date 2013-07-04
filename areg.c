/**********************************************************************/
/*                                                                    */
/*  areg.c: code for a register of pointers that are known to amalloc */
/*                                                                    */
/*  (C) Copyright 2013 Ramses van Zon                                 */
/*                                                                    */
/**********************************************************************/

#include <string.h>              /* for memmove and memcpy       */
#include <stdlib.h>              /* for malloc and realloc       */
#include <stdio.h>              /* for malloc and realloc       */

typedef const void* areg_key_t;   /* key type                    */
typedef size_t clue_t;            /* use clues for faster lookup */

#define NOCLUE ((clue_t)(0))     /* error/ignorance             */
#define AREG_SUCCESS   0          /* function call successful    */
#define AREG_FAILURE   1          /* an error occurred           */
#define AREG_NOT_FOUND 2          /*  entry not found            */

/* For the routines to be thread safe, use pthreads or openmp.  */
#if defined(AREG_PTHREAD_LOCK)

  #include <pthread.h>    
  static  pthread_mutex_t areg_mutex = PTHREAD_MUTEX_INITIALIZER;

#elif defined(AREG_OPENMP_LOCK) || defined(_OPENMP)

  #include <omp.h>               
  static  omp_lock_t       areg_mutex;

#endif


/* keys are stored in a resizeable array. */

#define nregmaxinit 512
#define nreginc     512

#ifndef STATIC
#define STATIC
#endif
static size_t  nregmax = nregmaxinit;
STATIC size_t  nreg    = 0;
static areg_key_t   keyreginit[nregmaxinit];
STATIC areg_key_t*  keyreg  = keyreginit;

/**********************************************************************/

static
int internal_areg_find(areg_key_t key, clue_t clue, size_t* index)
{
 /* Find the index where 'key' is stored, starting at position 'clue'
    (or at nreg/2 if clue=NOCLUE). If found, stores the index in
    'index' and returns AREG_SUCCESS, if not found, stores the
    insertion point in 'index' and returns AREG_NOT_FOUND.  Upon
    error, it returns AREG_FAILURE. */

    size_t start;
    size_t lowi;
    size_t midi;
    size_t highi;
    
    /* index must point somewhere */
    if (index == NULL) {
        return AREG_FAILURE; 
    }

    /* ensure start is reasonable */
    start = clue;
    if (start == NOCLUE) {
        start = nreg/2;
    }  else if (nreg!=0 && start >= nreg) {
        start = nreg-1;
    }

    /* the binary search algorithm below assumes lowi<highi, so we first
       need to catch corner cases: empty list, list of 1 */
    if (nreg==0) {
        *index = 0;
        return AREG_NOT_FOUND;
    } else if (nreg==1) {
        if (key > keyreg[0]) {
            *index = 1;
        } else {
            *index = 0;
        }
        if (keyreg[*index] == key) {
            return AREG_SUCCESS;
        } else {
            return AREG_NOT_FOUND;
        }
    } 

    /* it also assumes key lies between keyreg[0] and keyreg[nreg-1],
       so check if it is out of range first */
    if (key <= keyreg[0]) {
        *index = 0;
        if (keyreg[0] == key)
            return AREG_SUCCESS;
        else
            return AREG_NOT_FOUND;
    }
    if (keyreg[nreg-1] <= key) {
        if (keyreg[nreg-1] == key) {
            *index = nreg-1;
            return AREG_SUCCESS;
        } else {
            *index = nreg;
            return AREG_NOT_FOUND;
        }
    }

    /* setup binary search */
    lowi = 0;
    midi = start;    
    highi = nreg-1;   

    /* check the mid point first */
    if (keyreg[midi] == key) {
        *index = midi;
        return AREG_SUCCESS;
    }
    /* we get here if reg[lowi].key < key < reg[highi].key */
    /* start bisection */
    while (highi-lowi>1) {
        if (keyreg[midi] <= key ) 
            lowi = midi;
        else
            highi = midi;
        midi = (highi+lowi)/2;
        if (keyreg[midi] == key ) {
            *index = midi;
            return AREG_SUCCESS;
        }
    }
    /* result should be highi */
    *index=highi;
    return AREG_NOT_FOUND;
}

static
void internal_lock_on()
{
    #ifdef AREG_PTHREAD_LOCK
    pthread_mutex_lock(&areg_mutex);
    #elif defined(AREG_OPENMP_LOCK)
    static int areg_mutex_initialized = 0;
    if (areg_mutex_initialized) {
        omp_set_lock(&areg_mutex);
    } else {
        areg_mutex_initialized = 1;
        omp_init_lock(&areg_mutex);
        omp_set_lock(&areg_mutex);
    }
    #endif
}

static
void internal_lock_off()
{
    #ifdef AREG_PTHREAD_LOCK
    pthread_mutex_unlock(&areg_mutex);
    #elif defined(AREG_OPENMP_LOCK)
    omp_unset_lock(&areg_mutex);
    #endif
}

int areg_add(areg_key_t key, clue_t* clue) 
{
 /* Add a key and return a clue to where to find the key. Returns
    an error code, which is 0 if the addition was successful. If
    clue==NULL, no clue is given. */

    size_t       index  = 0;
    int          exitcode = AREG_FAILURE;
    int          notpresent;
    areg_key_t*  newkeyreg;
   
    if (key == NULL) {
        return AREG_FAILURE;
    }

    internal_lock_on();

    /* check if we need to augment the registry */
    if (nreg == nregmax) { 
        
        if (nregmax == nregmaxinit) {

            newkeyreg = malloc((nregmaxinit+nreginc)*sizeof(areg_key_t));
            if (newkeyreg!=NULL) {
              memcpy(newkeyreg, keyreg, nregmaxinit*sizeof(areg_key_t));
            }

        } else {           

            newkeyreg = realloc(keyreg, (nregmax+nreginc)*sizeof(areg_key_t));

        }

        if (newkeyreg == NULL) {           
            internal_lock_off();
            return AREG_FAILURE;
        }

        keyreg = newkeyreg;
        nregmax += nreginc;
    }

    /* find the ordered spot (gets put in index) */
    notpresent = ( internal_areg_find(key, NOCLUE, &index) 
                   == AREG_NOT_FOUND ); /* must not be found */
    if (notpresent) {
        /* index given nu internal_areg_find holds insertion point */
        /* clear the spot */
        if (index<nreg) {
            memmove(keyreg+index+1, keyreg+index, (nreg-index)*sizeof(areg_key_t));
        }
        nreg++;
        /* put it there */
        keyreg[index]=key;
        exitcode = AREG_SUCCESS;
    }


    if (clue != NULL) {
        if (exitcode != AREG_SUCCESS) {
            *clue = NOCLUE;
        } else {
            *clue = index;
        }
    }

    internal_lock_off();
    return exitcode;
}

/**********************************************************************/

int areg_remove(areg_key_t key, clue_t clue) 
{
 /* Remove the key 'key'.  Pass in a clue that was given by areg_add()
    for faster lookup.  Returns 0 if 'key' was removed and 1 if 'key'
    was not found. */

    fprintf(stdout,"Whaa!\n");
    int     exitcode;
    size_t  index;

    internal_lock_on();

    exitcode = internal_areg_find(key, clue, &index);
    if (exitcode == AREG_SUCCESS && index < nreg) {
        if (index < nreg-1)  
            memmove(keyreg+index, keyreg+index+1, (nreg-index-1)*sizeof(areg_key_t));
        nreg--;
    }    
    /* check if we can shrink the registry ?*/
    if (nreg < nregmax-(7*nreginc)/6)    /* can we spare the room? */
    {
        if (nregmax > nregmaxinit+nreginc) {
            /* We just have to reallocate the buffer */
            /* - determine the new maximum number of registrants */
            size_t newnregmax = nregmax-nreginc; 
            /* - perform the reallocation */
            areg_key_t* newkeyreg = realloc(keyreg, newnregmax*sizeof(areg_key_t));
            /* - check if successfull */
            if (newkeyreg != NULL) {
                /* - if so, reset the number of registrants */
                nregmax = newnregmax;
                /*   and the reg pointer */
                keyreg = newkeyreg;
            }
        } else if (nregmax == nregmaxinit+nreginc) {
            /* We get here if we have to shrink to initial buffer */
            /* - copy everything back to that initial buffer */
            memcpy(keyreginit, keyreg, nreg*sizeof(areg_key_t));
            /* - release memory taken by newer buffer */
            free(keyreg);
            /* - set the maximum number of registrants back */
            nregmax = nregmaxinit;
            /* - point the reg pointer back to the initial buffer */
            keyreg = keyreginit;
        }
    }
    internal_lock_off();
    return exitcode;
}

/**********************************************************************/

int areg_lookup(areg_key_t key, clue_t clue) 
{
 /* Looks for 'key'.  Pass in clue given by areg_add() for faster
    areg_lookup. Returns AREG_FAILURE if key is not found. */
    int exitcode;
    size_t index;
    if (key == NULL) 
        return AREG_FAILURE;
    internal_lock_on();
    exitcode = internal_areg_find(key, clue, &index);
    internal_lock_off();
    return exitcode;
}

/**********************************************************************/
