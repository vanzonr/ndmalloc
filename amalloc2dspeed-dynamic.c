/* 
 * amalloc2dspeed-dynamic.c - speed test for amalloc dynamic array library
 */

#include <stdlib.h>
#include "test_damalloc.h"
#include "pass.h"
#include "ndef.h"

double case_dyn(int repeat)
{
    int i, j;
    double d = 0.0;
    float** a = test_damalloc2d(sizeof(float), n, n);
    float** b = test_damalloc2d(sizeof(float), n, n);
    float** c = test_damalloc2d(sizeof(float), n, n);
    while (repeat--) {
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) {
                a[i][j] = i+repeat;
                b[i][j] = j+repeat/2;
            }
        pass(a[0],b[0],&repeat);
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) 
                c[i][j] = a[i][j]+b[i][j];
        pass(c[0],c[0],&repeat);
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) 
                d += c[i][j];
        pass(c[0],(float*)&d,&repeat);
    }
    test_dafree2d(a,n);
    test_dafree2d(b,n);
    test_dafree2d(c,n);
    return d;
}


