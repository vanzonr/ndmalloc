#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ndmalloc.h"

void fill(float** array)
{
    const size_t* shape = ndshape(array);
    float* data = nddata(array);
    int i;
    assert( ndisknown(array) );
    assert( ndrank(array) == 2 );
    for (i = 0; i < shape[0]*shape[1]; i++)
        data[i] = i+1;
}

void print(float** array)
{
    const size_t* shape = ndshape(array);
    int i,j;
    assert( ndisknown(array) );
    for (i = 0; i < shape[0]; i++) {
        for (j = 0; j < shape[1]; j++)
            printf("%.1f\t",array[i][j]);
        printf("\n");
    }
}

int main()
{
    float** a = ndmalloc(sizeof(float), 2, 4, 4);
    fill(a);
    print(a);

    float* data = nddata(a);
    data[1] = 100;

    float** b = ndrealloc(a, sizeof(float), 2, 9, 2);
    float** c = ndview(b, sizeof(float), 2, 3, 6);
    float d[3][5] = { {11,12,13,14,1},
                      {15,16,17,18,2},
                      {19,20,21,22,3} };
    float** e = autoview2(d);
    print(b);
    print(c);
    ndfree(b); /* not a! */
    ndfree(c); /* although it is a view, must deallocate */

    print(e);
    ndfree(e);

    return 0;
}
