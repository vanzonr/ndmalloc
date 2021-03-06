#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ndmalloc.h"
#include "errno.h"

/* it's a bit hard to use multidimensional arrays in a -correct
   way, but this is one approach */
#define ARRAY1D(type) type*
#define ARRAY2D(type) type**
#define ARRAY3D(type) type***
#define ARRAY4D(type) type****
#define ARRAY5D(type) type*****

void qerror(int i,  char*s) { errno=i; perror(s); exit(i); }

void fill( ARRAY1D(float) array)
{
    if ( ! ndisknown(array) ) 
        qerror(1,"Whoops");
    size_t  size = ndsize(array,0);
    size_t i;
    for (i = 0; i < size; i++)
        array[i] = i+1;
}

void print(  ARRAY3D(float) array)
{
    assert( ndisknown(array) );
    assert( ndrank(array) == 3 );
    const size_t * shape = ndshape(array);
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
    ARRAY3D(float) a = ndmalloc(sizeof(float), 3, 5,4,3);
    fill(nddata(a));
    print(a);
    ndfree(a);
    return 0;
}
