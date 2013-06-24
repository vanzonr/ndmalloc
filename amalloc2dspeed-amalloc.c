/* 
 * amalloc2dspeed-amalloc.c - speed test
 * for amalloc dynamic array library
 */

#include <stdlib.h>
#include "amalloc.h"
#include "pass.h"
#include "ndef.h"

double case_amalloc(int repeat)
{
    int i, j;
    double d = 0.0;
    float** a = amalloc(sizeof(float), 2, n, n);
    float** b = amalloc(sizeof(float), 2, n, n);
    float** c = amalloc(sizeof(float), 2, n, n);
    while (repeat--) {
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) {
                a[i][j] = i+repeat;
                b[i][j] = j+repeat/2;
            }
        pass(&a[0][0],&b[0][0],&repeat);
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) 
                c[i][j] = a[i][j]+b[i][j];
        pass(&c[0][0],&c[0][0],&repeat);
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) 
                d += c[i][j];
        pass(&c[0][0],(float*)&d,&repeat);
    }
    afree(a);
    afree(b);
    afree(c);
    return d;
}


