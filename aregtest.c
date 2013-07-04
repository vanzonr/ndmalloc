/**********************************************************************/
/*                                                                    */
/*  aregtest.c: test code for a register of pointers that are known to
    amalloc */
/*                                                                    */
/*  (C) Copyright 2013 Ramses van Zon                                 */
/*                                                                    */
/**********************************************************************/

#include "areg.h"

#define AREG_CHECK(x) if(x)fprintf(stderr,"Warning ("__FILE__":%d): '"#x"' failed!\n",__LINE__);

extern int nreg;
extern void** keyreg;

#include <stdio.h>
void ABC() 
{
    size_t i;
    for (i = 0; i < nreg; i++) {
        printf("%p\n", keyreg[i]);
    }
    printf("*****************\n");
}



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
    int* p=NULL;
    clue_t cluea;
    clue_t cluec;

    ABC();

    AREG_CHECK(  areg_add(pa, &cluea)  );
    ABC();
    AREG_CHECK(  areg_add(pc, &cluec)  );
    ABC();

    AREG_CHECK(  areg_remove(pc, cluec)   );
    AREG_CHECK(  areg_remove(pc, AREG_NOCLUE)  );
    ABC();

    #define nnumbers 1010000
    int x[nnumbers];
    int y[nnumbers];
    for (i=0;i<nnumbers;i++) {
        x[i]=i; y[i]=1000+i;
        if (i!=3) {
            AREG_CHECK(  areg_add(x+i,NULL)  );
        }
    }
    i=3;
    AREG_CHECK(  areg_add(x+i,NULL)  );

    i=3;
    AREG_CHECK(  areg_add(x+i,NULL)  );

    AREG_CHECK(  areg_lookup(pa, cluea)  );

    return p?*p:cluec*0;
}
