#include "amalloc.h"

int main()
{
    double** a = acalloc(sizeof(double), 2, 10, 4);
    a[9][3] = 7;
    double y=8;
    a[9][3] = y;
    double z = a[9][3]; 
    y = a[9][3];
    afree(a);
    return z+y;
}
