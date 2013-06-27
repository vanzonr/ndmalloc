/* test2d.c */

#include <stdio.h>
#include "amalloc.h"

/* Six different signatures of a print function for a matrix: */

/* print_1 takes a double-pointer matrix, whose elements and row
   pointers could be changed. Dangerous. */
void print_1(float**a, int n, int m);

/* print_2 takes a matrix with const elements, but whose row pointers
   could in principle be changed. Dangerous, but not uncommon! */
void print_2(const float**a, int n, int m);

/* print_3 takes a matrix, which is a pointer to a set of
   pointers. The row pointers are constant, but the elements would be
   changable.*/
void print_3(float *const* a, int n, int m);

/* print_4 takes a constant matrix, as a set of pointers to rows. Both
   the row pointers and the elements are const, and can't be changed. */
void print_4(const float*const*a, int n, int m);

/* print_5 wants the matrix as a contiguous memory block.  Dangerous,
   and very common.*/
void print_5(float *a, int n, int m);

/* print_6 wants the constant matrix as a contiguous memory block.
   because of const, print_6 cannot change the elements of a. */
void print_6(const float *a, int n, int m);

/* print_7 takes the wrapper 2d class, which already contains its dimenstions
 because of const, print_7 cannot change the elements of a. */
void print_7(float** a);

int main() 
{
    const int n = 9;
    const int m = 5;
    float** a = amalloc(sizeof(float),2,n,m);
    int i, j;

    for (i=0;i<n;i++)
        for (j=0;j<m;j++)
            a[i][j]=(i+1)*10+j+1;
    
    float* a1 = a[1];

    print_1(a, asize(a,0), asize(a,1));
    print_2((void*)a, asize(a,0), asize(a,1));
    print_3((void*)a, asize(a,0), asize(a,1));
    print_4((void*)a, asize(a,0), asize(a,1));
    print_5(adata(a), asize(a,0), asize(a,1));
    print_6(acdata(a), asize(a,0), asize(a,1));
    print_7(a);

    afree(a);
}

/************************************************/
void print_1(float**a, int n, int m) 
{
    int i, j;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) 
            printf("%.0f ", a[i][j]);
        printf("\n");
    }
    printf("\n");
}
/************************************************/
void print_2(const float**a, int n, int m) 
{
    int i, j;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) 
            printf("%.0f ", a[i][j]);
        printf("\n");
    }
    printf("\n");
}
/************************************************/
void print_3(float *const*const a, int n, int m) 
{
    int i, j;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) 
            printf("%.0f ", a[i][j]);
        printf("\n");
    }
    printf("\n");
}
/************************************************/
void print_4(const float*const*a, int n, int m) 
{
    int i, j;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) 
            printf("%.0f ", a[i][j]);
        printf("\n");
    }
    printf("\n");
}
/************************************************/
void print_5(float *a, int n, int m) 
{
    int i, j;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) 
            printf("%.0f ", a[i*m+j]);
        printf("\n");
    }
    printf("\n");
}
/************************************************/
void print_6(const float *a, int n, int m) 
{
    int i, j;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) 
            printf("%.0f ", a[i*m+j]);
        printf("\n");
    }
    printf("\n");
}
/************************************************/
void print_7(float** a) 
{
    int i, j;
    for (i=0;i<asize(a,0);i++) {
        for (j=0;j<asize(a,1);j++) 
            printf("%.0f ", a[i][j]);
        printf("\n");
    }
    printf("\n");
}
/************************************************/
