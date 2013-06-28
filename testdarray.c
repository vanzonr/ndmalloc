#include "amalloc.h"

int main()
{
    double** a = acalloc(sizeof(double), 2, 10, 4);
    double   y = 8;
    double   z = a[9][3]; 
    a[9][3] = 7;
    a[9][3] = y;
    y = a[9][3];
    afree(a);
    return z+y;
}
