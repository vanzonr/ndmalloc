/* 
 * ndmalloc2dspeed-auto.c - speed test for automatic array
 */

#include <stdlib.h>
#include "optbarrier.h"
#include "ndef.h"

double case_auto(int repeat) 
{
    int i, j;
    double d = 0.0;
    float a[n][n];
    float b[n][n];
    float c[n][n];
    while (repeat--) {
        for ( i=0;i<n;i++)
            for (j=0;j<n;j++) {
                a[i][j] = i+repeat;
                b[i][j] = j+repeat/2;
            }
        optbarrier(a[0],b[0],&repeat);
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) 
                c[i][j] = a[i][j]+b[i][j];
        optbarrier(c[0],c[0],&repeat);
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) 
                 d += c[i][j];
        optbarrier(c[0],(float*)&d,&repeat);
    }
    return d;
}

