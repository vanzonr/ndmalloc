#include <stdio.h>
#include "darray.h"

int main()
{
  darray  _a = dcalloc(sizeof(double), 2, 10, 4);
  double** a = (double**)(_a.array);
  // ^- should become 'double a[*][*] = dmalloc(sizeof(double), 2, 10, 4);'
  a[9][3] = 7;
  double y=8;
  dset(&y, _a, 9, 3);
  double z = a[9][3]; 
  dget(&y, _a, 9, 3);
  dfree(_a);
  // ^- should become 'dfree(a);'
  printf("%d\n",getpagesize());
  return z+y;
}
