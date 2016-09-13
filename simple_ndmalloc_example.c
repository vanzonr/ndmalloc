/* This simple example for ndmalloc creates an array of rank 2
  (i.e. a matrix) with dimensions 4x6, set a few elements, and print
  the matrix. */
#include <stdio.h>
#include <assert.h>
#include <ndmalloc.h>
void print_matrix(int** m)
{
    int i, j;
    for (i = 0; i < ndsize(m,0); ++i) {
        for (j = 0; j < ndsize(m,1); ++j) {
            printf("%d ", m[i][j]);
        }
        printf("\n");
    }
}
int main()
{
    int** m = ndcalloc(sizeof(int),2,4,6);
    m[0][0] = 5;
    m[3][5] = 1;
    print_matrix(m);
    ndfree(m);
}
