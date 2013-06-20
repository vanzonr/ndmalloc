#include <stdio.h>
#include "amalloc.h"
#include "damalloc.h"

int main()
{
    darray  _a = dacalloc(sizeof(double), 2, 10, 4);
    double** a = (double**)(_a.array);
    // ^- should become 'double a[*][*] = dmalloc(sizeof(double), 2, 10, 4);'
    a[9][3] = 7;
    double y=8;
    daset(&y, _a, 9, 3);
    double z = a[9][3]; 
    daget(&y, _a, 9, 3);
    dafree(_a);
    // ^- should become 'dfree(a);'
    //  printf("%d\n",getpagesize());
    return z+y;
}
