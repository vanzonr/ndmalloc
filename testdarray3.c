#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "amalloc.h"

void fill(float** array)
{
    const size_t* shape = ashape(array);
    float* data = adata(array);
    int i;
    assert( aknown(array) );
    assert( arank(array) == 2 );
    for (i = 0; i < shape[0]*shape[1]; i++)
        data[i] = i+1;
}

void print(float** array)
{
    const size_t* shape = ashape(array);
    int i,j;
    assert( aknown(array) );
    for (i = 0; i < shape[0]; i++) {
        for (j = 0; j < shape[1]; j++)
            printf("%.1f\t",array[i][j]);
        printf("\n");
    }
}

int main()
{
    float** a = amalloc(sizeof(float), 2, 4, 4);
    float** b = arealloc(a, sizeof(float), 2, 9, 2);
    float** c = aview(b, sizeof(float), 2, 3, 6);
    float d[3][5] = { {11,12,13,14,1},
                      {15,16,17,18,2},
                      {19,20,21,22,3} };
    float** e = autoview2(d);
    fill(a);
    float* data = adata(a);
    data[1] = 100;
    print(a);
    print(b);
    print(c);
    afree(b); /* not a! */
    afree(c); /* although it is a view, must deallocate */

    print(e);
    afree(e);

    return 0;
}
