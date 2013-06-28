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
    double** c = aview(b, sizeof(double), 2, 3, 6);
    print(c);
    afree(b); // not a!
    afree(c); // although it is a view, must deallocate

    double d[3][4] = { {11,12,13,14},
                       {15,16,17,18},
                       {19,20,21,22} };
    double** e = aview(d, sizeof(double), 2, 3, 4);
    print(e);
    afree(e);

    return 0;
}
