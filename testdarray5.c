#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "amalloc.h"

void fill(float*** array)
{
    assert( aknown(array) );
    float* const data = adata(array);
    size_t const size = afullsize(array);
    int i;
    for (i = 0; i < size; i++)
        data[i] = i+1;
}

void print(float*** array)
{
    assert( aknown(array) );
    assert( arank(array) == 3 );
    size_t const*const shape = ashape(array);
    int i,j,k;
    for (i = 0; i < shape[0]; i++) {
        for (j = 0; j < shape[1]; j++) {
            printf("%*s",4*i,"");
            for (k = 0; k < shape[2]; k++) {
                printf("%8.2f ",array[i][j][k]);
            }
            printf("\n");
        }
        printf("\n");
    }
    printf("\n");
}

int main()
{
    float*** a = amalloc(sizeof(float), 3, 5,4,3);
    fill(a);

    float* data = adata(a);
    data[1] = 100;

    print(a);

    float*** b = arealloc(a, sizeof(float), 3, 9,2,2);
    print(b);
  
    float*** c = aview (b, sizeof(float), 3, 3,5,2);
    print(c);

    float d[3][2][5] = { {{11,12,13,14,1},{11,12,13,14,1}},
                          {{15,16,17,18,2},{15,16,17,18,2}},
                          {{19,20,21,22,3},{19,20,21,22,3}} };
    float*** e = autoview3(d);

    print(e);

    afree(b);
    afree(c); /* although c and e are aviews, must deallocate */
    afree(e);

    return 0;
}
