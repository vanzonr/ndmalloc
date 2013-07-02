/**********************************************************************/
/*                                                                    */
/*  aregister.c: design code for a register for pointer               */
/*               to be used with amalloc.                             */
/*                                                                    */
/*  (C) Copyright 2013 Ramses van Zon                                 */
/*                                                                    */
/**********************************************************************/

#include <pthread.h>    /* for thread locks */
#include <string.h>     /* for memmove and memcpy */
#include <stdlib.h>     /* for malloc and realloc */
#include "aregister.h"

/**********************************************************************/

/* Basic search item */
struct keyval {
    const void* key;     /* what to search for (T***)    */
          void* val;     /* what to  find (header_t*)    */
};

typedef size_t clue_t;   /* use clues for faster lookup */

#define NOCLUE        ((clue_t)(-1)) /* error/ignorance */
#define AREG_SUCCESS  0  /* function call successful    */
#define AREG_FAILURE  1  /* an error occurred           */
#define AREG_ERROR    1  /* other name for AREG_FAILURE */

/**********************************************************************/

/* For the routines to be thread safe, using pthreads.  */

static pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;

/* keyval's should be in a B-tree. For now, use a fixed size array. */

#define nregmaxinit 512
#define nreginc     512

static size_t          nregmax = nregmaxinit;
static size_t          nreg  = 0;
static struct keyval   reginit[nregmaxinit];
static struct keyval*  reg = reginit;

/**********************************************************************/

#include <stdio.h>
void ABC() 
{
    size_t i;
    for (i = 0; i < nreg; i++) {
        printf("%p %p %d\n", 
               reg[i].key, 
               reg[i].val, 
               *(int*)(reg[i].val));
    }
    printf("*****************\n");
}

/**********************************************************************/

static inline
int internal_areg_find(const void* key, clue_t clue, size_t* index)
{
 /* Find the index where 'key' is stored, starting at position 'clue'
    (or at nreg/2 if clue=NOCLUE). If found, store the index in
    'index' and return AREG_SUCCESS, if not found, stores the
    insertion point in 'index' and returns AREG_FAILURE.  */

    size_t  start = clue;
    size_t  i;

    if (index == NULL) {
        return AREG_FAILURE;
    }

    if (start == NOCLUE) {
        start = 0;
    }
    if (nreg!=0 && start >= nreg) {
        start = nreg-1;
    }

    for (i = start; i < nreg; i++) {
        if (reg[i].key >= key) {
            *index = i;
            if (reg[i].key == key) {
                return AREG_SUCCESS;
            } else {
                return AREG_FAILURE;
            }
        }
    }
    for (i = 0; i < start; i++) {
        if (reg[start-i].key <= key) {
            if (reg[start-i].key == key) {
                *index = start-i+1;
                return AREG_SUCCESS;
            } else {
                *index = start-i;
                return AREG_FAILURE;
            } 
        }    
    }

    *index = nreg;
    return AREG_FAILURE;
}

int areg_add(const void* key, void* val, clue_t* clue) 
{
 /* Add a (key, value) and return a clue to where to find key. Returns
    and error code, which is 0 if the addition was successful. */

    size_t   index  = 0;
    int      exitcode = AREG_FAILURE;
    int      notpresent;
    struct keyval* newreg;
    

    if (key == NULL || val == NULL) {
        return AREG_FAILURE;
    }

    pthread_mutex_lock(&mutex);

    /* check if we need to augment the registry */
    if (nreg == nregmax) { 

        
        if (nregmax == nregmaxinit) {

            newreg = malloc((nregmaxinit+nreginc)*sizeof(struct keyval));
            if (newreg)
                memcpy(newreg, reg, nregmaxinit*sizeof(struct keyval));

        } else {           

            newreg = realloc(reg, (nregmax+nreginc)*sizeof(struct keyval));

        }

        if (newreg == NULL) {                
            pthread_mutex_unlock(&mutex);
            return AREG_ERROR;
        }

        fprintf(stderr, "Yeah!");
        reg = newreg;
        nregmax += nreginc;
    }

    /* find the ordered spot (gets put in index) */
    notpresent = ( internal_areg_find(key, NOCLUE, &index) 
                   == AREG_FAILURE ); /* must not be found */
    if (notpresent) {
        /* index given nu internal_areg_find holds insertion point */
        /* clear the spot */
        if (index<nreg) 
            memmove(reg+index+1, 
                    reg+index, 
                    (nreg-index)*sizeof(struct keyval));
        nreg++;
        /* put it there */
        reg[index].key=key;
        reg[index].val=val;
        exitcode = AREG_SUCCESS;
    }


    if (clue != NULL) {
        if (exitcode != AREG_SUCCESS) {
            *clue = NOCLUE;
        } else {
            *clue = index;
        }
    }

    pthread_mutex_unlock(&mutex);
    return exitcode;
}

/**********************************************************************/

int areg_remove(const void* key, clue_t clue) 
{
 /* Remove the key-value pair associated with 'key'.  Pass in a clue
    that was given by areg_add() for faster lookup.  Returns 0 if pair is
    removed and 1 if 'key' was not found. */

    int     exitcode;
    size_t  index;

    pthread_mutex_lock(&mutex);

    exitcode = internal_areg_find(key, clue, &index);
    if (exitcode == AREG_SUCCESS && index < nreg) {
        if (index < nreg-1)  
            memmove(reg+index, reg+index+1, 
                    (nreg-index-1)*sizeof(struct keyval));
        nreg--;
    }
    
    /* check if we can shrink the registry ?*/
    if (nregmax > nregmaxinit+nreginc // don't shrink to original buffer for now
        && nreg < nregmax-nreginc)    // must not need the room
    {
        size_t newnregmax = nregmax-nreginc; 
        struct keyval* newreg = realloc(reg, newnregmax*sizeof(struct keyval));
        if (newreg != NULL) {
            nregmax = newnregmax;
            reg = newreg;
        }
    }

    pthread_mutex_unlock(&mutex);
    return exitcode;
}

/**********************************************************************/

int areg_lookup(const void* key, clue_t clue, void** value) 
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

    pthread_mutex_lock(&mutex);

    exitcode = internal_areg_find(key, clue, &index);

    if ( exitcode == AREG_SUCCESS ) {
        *value = reg[index].val;
    } else {
        *value = NULL;
    }

    pthread_mutex_unlock(&mutex);
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
    clue_t clueb;
    clue_t cluec;
    clue_t clued;
    void* result;

    ABC();

    AREG_CHECK(  areg_add(pa, pb, &cluea)  );
    ABC();
    AREG_CHECK(  areg_add(pc, pd, &cluec)  );
    ABC();

    AREG_CHECK(  areg_remove(pc, cluec)   );
    AREG_CHECK(  areg_remove(pc, NOCLUE)  );
    ABC();
    
    for (i=0;i<1024;i++) {
        int* a=malloc(sizeof(int));        
        int* b=malloc(sizeof(int));
        *a=i; *b=1000+i;
        clue_t clue;
        AREG_CHECK(  areg_add(a,b,&clue)  );
    }

    ABC();

    AREG_CHECK(  areg_lookup(pa, cluea, &result)  );
    p = result;

    return p?*p:cluec*0;
}
