#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "amalloc.h"

void fill(double** array)
{
    assert(aknown(array));
    const int n = asize(array,0);
    const int m = asize(array,1);
    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            array[i][j] = i + j;
}

int main()
{
    double** a = amalloc(sizeof(double), 2, 10, 4);
    fill(a);
    double z = a[9][3]; 
    afree((void*)a);
    double** b = calloc(10,sizeof(double*));
    int k;
    for(k=0;k<10;k++)
        b[k]=calloc(4,sizeof(double));
    fill(b);
    return z;
}
