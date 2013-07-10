#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ndmalloc.h"

void fill(float*** array)
{
    const size_t* const shape = ndshape(array);
    float* data = nddata(array);
    int i;
    assert( ndisknown(array) );
    assert( ndrank(array) == 3 );
    for (i = 0; i < shape[0]*shape[1]*shape[2]; i++)
        data[i] = i+1;
}

void print(float*** array)
{
    const size_t* const shape = ndshape(array);
    int i,j,k;
    assert( ndisknown(array) );
    assert( ndrank(array) == 3 );
    for (i = 0; i < shape[0]; i++) {
        for (j = 0; j < shape[1]; j++) {
            for (k = 0; k < shape[2]; k++) {
                printf("%.1f\t",array[i][j][k]);
            }
            printf("\n");
            for (j = 0; j < shape[1]; j++) 
                printf(" ");
        }
        printf("\n");
    }
}

int main()
{
    float*** a = ndmalloc(sizeof(float), 3, 4,2,2 );
    float*** b = ndrealloc(a, sizeof(float), 3, 2,4,3 );
    float*** c = ndview(b, sizeof(float), 3, 3,4,2);
    float d[3][2][5] = { {{11,12,13,14,1},{11,12,13,14,1}},
                         {{15,16,17,18,2},{15,16,17,18,2}},
                         {{19,20,21,22,3},{19,20,21,22,3}} };
    float*** e = autoview3(d);
    fill(a);
    float* data = nddata(a);
    data[1] = 100;
    print(a);
    print(b);
    print(c);
    ndfree(b); /* not a! */
    ndfree(c); /* although it is a view, must deallocate */

    print(e);
    ndfree(e);

    return 0;
}
