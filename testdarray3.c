#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "amalloc.h"

void fill(double** array)
{
    assert( aknown(array) );
    assert( arank(array) == 2 );
    const size_t* shape = ashape(array);
    double* data = adata(array);
    int i;
    for (i = 0; i < shape[0]*shape[1]; i++)
        data[i] = i+1;
}

void print(double** array)
{
    assert( aknown(array) );
    const size_t* shape = ashape(array);
    int i,j;
    for (i = 0; i < shape[0]; i++) {
        for (j = 0; j < shape[1]; j++)
            printf("%.1lf\t",array[i][j]);
        printf("\n");
    }
}

int main()
{
    double** a = amalloc(sizeof(double), 2, 4, 4);
    fill(a);
    print(a);
    double** b = arealloc(a, sizeof(double), 2, 9, 2);
    print(b);
    afree(b); // not a!
    return 0;
}
