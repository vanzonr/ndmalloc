#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "darray.h"

void fill(double** array)
{
  assert(isdarray(array));
  const int n = extentof(array,0);
  const int m = extentof(array,1);
  int i, j;
  for (i = 0; i < n; i++)
    for (j = 0; j < m; j++)
      array[i][j] = i + j;
}

int main()
{
  double** a = amalloc(sizeof(double), 2, 10, 4);
  //double** a=calloc(10,sizeof(double*));
  int k;
  for(k=0;k<10;k++)
    a[k]=calloc(4,sizeof(double));
  fill(a);
  double z = a[9][3]; 
  afree(a);
  return z;
}
