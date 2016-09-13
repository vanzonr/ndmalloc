/* 
 * ndmalloc2dspeed-ndmalloc.c - speed test
 * for ndmalloc dynamic array library
 */

#include <stdlib.h>
#include "ndmalloc.h"
#include "optbarrier.h"
#include "ndef.h"

double case_ndmalloc(int repeat)
{
    int i, j;
    double d = 0.0;
    float** a = ndmalloc(sizeof(float), 2, n, n);
    float** b = ndmalloc(sizeof(float), 2, n, n);
    float** c = ndmalloc(sizeof(float), 2, n, n);
    while (repeat--) {
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) {
                a[i][j] = i+repeat;
                b[i][j] = j+repeat/2;
            }
        optbarrier(&a[0][0],&b[0][0],&repeat);
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) 
                c[i][j] = a[i][j]+b[i][j];
        optbarrier(&c[0][0],&c[0][0],&repeat);
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) 
                d += c[i][j];
        optbarrier(&c[0][0],(float*)&d,&repeat);
    }
    ndfree(a);
    ndfree(b);
    ndfree(c);
    return d;
}


