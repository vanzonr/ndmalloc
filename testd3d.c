#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ndmalloc.h"
#include "errno.h"

#define ARRAY1D(type) type*
#define ARRAY2D(type) type*const*
#define ARRAY3D(type) type*const*const*
#define ARRAY4D(type) type*const*const*const*
#define ARRAY5D(type) type*const*const*const*const*

void qerror(int i, const char*s) { errno=i; perror(s); exit(i); }

void fill( ARRAY1D(float) array)
{
    if ( ! ndisknown(array) ) 
        qerror(1,"Whoops");
    size_t const size = ndsize(array,0);
    size_t i;
    for (i = 0; i < size; i++)
        array[i] = i+1;
}

void print( const ARRAY3D(float) array)
{
    assert( ndisknown(array) );
    assert( ndrank(array) == 3 );
    size_t const*const shape = ndshape(array);
    size_t i,j,k;
    for (i = 0; i < shape[0]; i++) {
        for (j = 0; j < shape[1]; j++) {
            printf("%*s",(int)(4*i),"");
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
    ARRAY3D(float) a = (float*const*const*)ndmalloc(sizeof(float), 3, 5,4,3);
    fill((float*)nddata((float***)a));
    print(a);
    ndfree((float***)a);
    return 0;
}
