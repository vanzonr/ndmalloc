#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "darray.h"

void fill(double** array)
{
  assert(paisda(array));
  const int n = paextentof(array,0);
  const int m = paextentof(array,1);
  int i, j;
  for (i = 0; i < n; i++)
    for (j = 0; j < m; j++)
      array[i][j] = i + j;
}

int main()
{
  double** a = pamalloc(sizeof(double), 2, 10, 4);
  fill(a);
  double z = a[9][3]; 
  pafree(a);
  double** b = calloc(10,sizeof(double*));
  int k;
  for(k=0;k<10;k++)
    b[k]=calloc(4,sizeof(double));
  fill(b);
  return z;
}
