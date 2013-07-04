/**********************************************************************/
/*                                                                    */
/*  aregister.c: design code for a register for pointer               */
/*               to be used with amalloc.                             */
/*                                                                    */
/*  (C) Copyright 2013 Ramses van Zon                                 */
/*                                                                    */
/**********************************************************************/

#include <string.h>           /* for memmove and memcpy */
#include <stdlib.h>           /* for malloc and realloc */

#if defined(AREG_PTHREAD_LOCK)
#include <pthread.h>          /* for thread locks */
#elif defined(AREG_OPENMP_LOCK)
#include <omp.h>              /* for openmp locks */
#endif

/*#include "aregister.h"*/

/**********************************************************************/

/* Basic search item */

#ifndef areg_val_t
#define areg_val_t void*           /* use -Dareg_val_t=void* on cmdline */
#endif

typedef const void* areg_key_t;    /* key type                    */
typedef size_t clue_t;        /* use clues for faster lookup */

#define NOCLUE ((clue_t)(-1)) /* error/ignorance             */
#define AREG_SUCCESS   0      /* function call successful    */
#define AREG_FAILURE   1      /* an error occurred           */
#define AREG_ERROR     1      /* other name for AREG_FAILURE */
#define AREG_NOT_FOUND 2      /*  entry not found            */

/**********************************************************************/

/* For the routines to be thread safe, use pthreads or openmp.  */

#if defined(AREG_PTHREAD_LOCK)
static pthread_mutex_t areg_mutex = PTHREAD_MUTEX_INITIALIZER;
#elif defined(AREG_OPENMP_LOCK)
static omp_lock_t      areg_mutex;
#endif


/* keyval's should be in a B-tree. For now, use a resizeable array. */

#define nregmaxinit 512
#define nreginc     512

static size_t  nregmax = nregmaxinit;
static size_t  nreg    = 0;
static areg_key_t   keyreginit[nregmaxinit];
static areg_val_t   valreginit[nregmaxinit];
static areg_key_t*  keyreg  = keyreginit;
static areg_val_t*  valreg  = valreginit;

/**********************************************************************/

#include <stdio.h>
void ABC() 
{
    size_t i;
    for (i = 0; i < nreg; i++) {
        printf("%p %p %d\n", 
               keyreg[i], 
               valreg[i], 
               *(int*)(valreg[i]));
    }
    printf("*****************\n");
}

/**********************************************************************/

static inline
int internal_areg_find(areg_key_t key, clue_t clue, size_t* index)
{
 /* Find the index where 'key' is stored, starting at position 'clue'
    (or at nreg/2 if clue=NOCLUE). If found, store the index in
    'index' and return AREG_SUCCESS, if not found, stores the
    insertion point in 'index' and returns AREG_NOT_FOUND.  Upon
    error, it returns AREG_ERROR. */

    size_t  start = clue;

    if (index == NULL) {
        return AREG_FAILURE;
    }

    if (start == NOCLUE) {
        start = 0;
    }
    if (nreg!=0 && start >= nreg) {
        start = nreg-1;
    }

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
    
    /* binary search */
    size_t lowi = 0;
    size_t highi = nreg-1;
    size_t nowi = start;

    if (keyreg[nowi] == key) {
        *index = nowi;
        return AREG_SUCCESS;
    }
    if (keyreg[lowi] == key) {
        *index = lowi;
        return AREG_SUCCESS;
    }
    if (keyreg[lowi] > key) {
        *index = lowi;
        return AREG_NOT_FOUND;
    }
    if (keyreg[highi] == key) {
        *index = highi;
        return AREG_SUCCESS;
    }
    if (keyreg[highi] < key) {
        *index = highi+1;
        return AREG_NOT_FOUND;
    }
    /* we get here if reg[lowi].key < key < reg[highi].key */
    /* start bisection */
    while (highi-lowi>1) {
        if (keyreg[nowi] <= key ) 
            lowi = nowi;
        else
            highi = nowi;
        nowi = (highi+lowi)/2;
        if (keyreg[nowi] == key ) {
            *index = nowi;
            return AREG_SUCCESS;
        }
    }
    /* result should be highi */
    *index=highi;
    return AREG_NOT_FOUND;
}

static inline
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

static inline
void internal_lock_off()
{
    #ifdef AREG_PTHREAD_LOCK
    pthread_mutex_unlock(&areg_mutex);
    #elif defined(AREG_OPENMP_LOCK)
    omp_unset_lock(&areg_mutex);
    #endif
}

int areg_add(areg_key_t key, areg_val_t val, clue_t* clue) 
{
 /* Add a (key, value) and return a clue to where to find key. Returns
    and error code, which is 0 if the addition was successful. If
    clue==NULL, no clue is given. */

    size_t       index  = 0;
    int          exitcode = AREG_FAILURE;
    int          notpresent;
    areg_key_t*  newkeyreg;
    areg_val_t*  newvalreg;
    

    if (key == NULL || val == NULL) {
        return AREG_FAILURE;
    }

    internal_lock_on();

    /* check if we need to augment the registry */
    if (nreg == nregmax) { 

        
        if (nregmax == nregmaxinit) {

            newkeyreg = malloc((nregmaxinit+nreginc)*sizeof(areg_key_t));
            if (newkeyreg!=NULL) {
                newvalreg = malloc((nregmaxinit+nreginc)*sizeof(areg_val_t));
                if (newvalreg==NULL) {
                    free(newkeyreg);
                    newkeyreg = NULL;
                } else {
                    memcpy(newkeyreg, keyreg, nregmaxinit*sizeof(areg_key_t));
                    memcpy(newvalreg, valreg, nregmaxinit*sizeof(areg_val_t));
                }
            }

        } else {           

            newkeyreg = realloc(keyreg, (nregmax+nreginc)*sizeof(areg_key_t));
            if (newkeyreg!=NULL) {
                newvalreg = realloc(valreg, (nregmax+nreginc)*sizeof(areg_val_t));
                if (newvalreg == NULL) {
                    free(newkeyreg);
                    newkeyreg = NULL;
                }
            }

        }

        if (newkeyreg == NULL) {           
            internal_lock_off();
            return AREG_ERROR;
        }

        keyreg = newkeyreg;
        valreg = newvalreg;
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
            memmove(valreg+index+1, valreg+index, (nreg-index)*sizeof(areg_val_t));
        }
        nreg++;
        /* put it there */
        keyreg[index]=key;
        valreg[index]=val;
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
 /* Remove the key-value pair associated with 'key'.  Pass in a clue
    that was given by areg_add() for faster lookup.  Returns 0 if pair is
    removed and 1 if 'key' was not found. */

    int     exitcode;
    size_t  index;

    internal_lock_on();

    exitcode = internal_areg_find(key, clue, &index);
    if (exitcode == AREG_SUCCESS && index < nreg) {
        if (index < nreg-1)  {
            memmove(keyreg+index, keyreg+index+1, (nreg-index-1)*sizeof(areg_key_t));
            memmove(valreg+index, valreg+index+1, (nreg-index-1)*sizeof(areg_val_t));
        }
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
            areg_val_t* newvalreg = realloc(valreg, newnregmax*sizeof(areg_val_t));
            /* - check if successfull */
            if (newkeyreg != NULL) {
                /* - if so, reset the number of registrants */
                nregmax = newnregmax;
                /*   and the reg pointer */
                keyreg = newkeyreg;
                valreg = newvalreg;
            }
        } else if (nregmax == nregmaxinit+nreginc) {
            /* We get here if we have to shrink to initial buffer */
            /* - copy everything back to that initial buffer */
            memcpy(keyreginit, keyreg, nreg*sizeof(areg_key_t));
            memcpy(valreginit, valreg, nreg*sizeof(areg_val_t));
            /* - release memory taken by newer buffer */
            free(keyreg);
            free(valreg);
            /* - set the maximum number of registrants back */
            nregmax = nregmaxinit;
            /* - point the reg pointer back to the initial buffer */
            keyreg = keyreginit;
            keyreg = keyreginit;
        }
    }

    internal_lock_off();
    return exitcode;
}

/**********************************************************************/

int areg_lookup(areg_key_t key, clue_t clue, areg_val_t* value) 
{
 /* Returns the value associated with 'key'.  Pass in clue given by
    areg_add() for faster areg_lookup. Returns AREG_FAILURE if key is
    not found, and sets value to NULL. */

    int     exitcode;
    size_t  index;

    if ( key == NULL || value == NULL ) {
        if (value != NULL) {
            *value = NULL;
        }
        return AREG_FAILURE;
    }

    internal_lock_on();

    exitcode = internal_areg_find(key, clue, &index);

    if ( exitcode == AREG_SUCCESS ) {
        *value = valreg[index];
    } else {
        *value = NULL;
    }

    internal_lock_off();
    return exitcode;
}

/**********************************************************************/

#define AREG_CHECK(x) if(x)fprintf(stderr,"Warning ("__FILE__":%d): '"#x"' failed!\n",__LINE__);

int main()
{
    int i;
    int  a=1;
    int  b=123;
    int  c=3;
    int  d=4;
    int* pa = &a;
    int* pb = &b;
    int* pc = &c;
    int* pd = &d;
    int* p;
    clue_t cluea;
    clue_t cluec;
    areg_val_t result;

    ABC();

    AREG_CHECK(  areg_add(pa, pb, &cluea)  );
    ABC();
    AREG_CHECK(  areg_add(pc, pd, &cluec)  );
    ABC();

    AREG_CHECK(  areg_remove(pc, cluec)   );
    AREG_CHECK(  areg_remove(pc, NOCLUE)  );
    ABC();

    #define nnumbers 1010000
    int x[nnumbers];
    int y[nnumbers];
    for (i=0;i<nnumbers;i++) {
        x[i]=i; y[i]=1000+i;
        if (i!=3) {
            AREG_CHECK(  areg_add(x+i,y+i,NULL)  );
        }
    }
    i=3;
    AREG_CHECK(  areg_add(x+i,y+i,NULL)  );

    i=3;
    AREG_CHECK(  areg_add(x+i,y+i,NULL)  );

    AREG_CHECK(  areg_lookup(pa, cluea, &result)  );
    p = result;

    return p?*p:cluec*0;
}
