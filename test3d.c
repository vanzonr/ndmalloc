/* test3d.c */

#include <stdio.h>
#include "amalloc.h"

/* Six different signatures of a print function for a tensor: */

/*
 * print_1 takes a double-pointer tensor, whose elements and row
 * pointers could be changed. Dangerous.
 * Not const-correct, but common in non-const libraries.  
 */
void print_1(float***a, int n, int m, int l);

/*
 * print_2 takes a tensor whose elements are constant, but whose row
 * pointers could in principle be changed. Dangerous, but common!
 * Not const-correct.
*/
void print_2(const float***a, int n, int m, int l);

/*
 * print_3 takes a tensor, which is a pointer to a set of pointers. The
 * row pointers are constant, but the elements would be changable.
 */
void print_3(float *const*const* a, int n, int m, int l);

/*
 * print_4 takes a constant tensor, as a set of pointers to rows. Both
 * the row pointers and the elements are const, and can't be changed.
 * Const-correct.
 */
void print_4(const float*const*const*a, int n, int m, int l);

/*
 * print_5 wants the tensor as a contiguous memory block.
 * because of const, print_5 couldt change the elements of a. 
 * Dangerous, and very common.
 */
void print_5(float *a, int n, int m, int l);

/*
 * print_6 wants the constant tensor as a contiguous memory block.
 * because of const, print_6 cannot change the elements of a. 
 * Const-correct.
 */
void print_6(const float *a, int n, int m, int l);

/*
 * print_7 takes a amalloc'ed pointer, which already contains its dimensions.
 * Not const-correct.
 */
void print_7(float*** a);

/* (implementations at the end) */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */


int main() 
{
    const int n = 9;
    const int m = 5;
    const int l = 2;
    int i,j,k;
    float*** a = amalloc(sizeof(float),3, n, m, l);

    for (i=0;i<n;i++)
      for (j=0;j<m;j++)
        for (k=0;k<l;k++)
          a[i][j][k]=((i+1)*10+j+1)*10+k+1;
    
    print_1(a, asize(a,0), asize(a,1), asize(a,2)); 
    print_2((void*)a, asize(a,0), asize(a,1), asize(a,2));
    print_3((void*)a, asize(a,0), asize(a,1), asize(a,2));
    print_4((void*)a, asize(a,0), asize(a,1), asize(a,2));
    print_5(adata(a), asize(a,0), asize(a,1), asize(a,2));
    print_6(acdata(a), asize(a,0), asize(a,1), asize(a,2));
    print_7(a);

    afree(a);

    float b[n][m][l];
    for (i=0;i<n;i++)
      for (j=0;j<m;j++)
        for (k=0;k<l;k++)
          b[i][j][k]=((i+1)*10+j+1)*10+k+1;

    return b[0][0][0];

    /* print_1(make_shape(b).cptr(),n,m,l); */
    /* print_2(make_shape(b).cref().cptr(),n,m,l); */
    /* print_3(make_shape(b).ptr(),n,m,l); */
    /* print_4(make_shape(b).ptr(),dim_shape(b,0),dim_shape(b,1),dim_shape(b,2)); */
    /* print_5(b[0][0],n,m,l); */
    /* print_6(b[0][0],n,m,l); */
    /* print_7(make_shape(b)); */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void print_1(float***a, int n, int m, int l) 
{
    int i, j, k;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) {
            for (k=0;k<l;k++) 
                printf("%.0f ", a[i][j][k]);
            printf("      \t");
        }
        printf("\n");
    }
    printf("\n");
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void print_2(const float***a, int n, int m, int l) 
{
    int i, j, k;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) {
            for (k=0;k<l;k++) 
                printf("%.0f ", a[i][j][k]);
            printf("      \t");
        }
        printf("\n");
    }
    printf("\n");
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void print_3(float *const*const*const a, int n, int m, int l) 
{
    int i, j, k;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) {
            for (k=0;k<l;k++) 
                printf("%.0f ", a[i][j][k]);
            printf("      \t");
        }
        printf("\n");
    }
    printf("\n");
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void print_4(const float*const*const*a, int n, int m, int l) 
{
    int i, j, k;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) {
            for (k=0;k<l;k++) 
                printf("%.0f ", a[i][j][k]);
            printf("      \t");
        }
        printf("\n");
    }
    printf("\n");
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void print_5(float *a, int n, int m, int l) 
{
    int i, j, k;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) {
            for (k=0;k<l;k++) 
                printf("%.0f ", a[(i*m+j)*l+k]);
            printf("      \t");
        }
        printf("\n");
    }
    printf("\n");
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void print_6(const float *a, int n, int m, int l) 
{
    int i, j, k;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) {
            for (k=0;k<l;k++) 
                printf("%.0f ", a[(i*m+j)*l+k]);
            printf("      \t");
        }
        printf("\n");
    }
    printf("\n");
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void print_7(float*** a) 
{
    const int n = asize(a,0);
    const int m = asize(a,1);
    const int l = asize(a,2);
    int i, j, k;
    for (i=0;i<n;i++) {
        for (j=0;j<m;j++) {
            for (k=0;k<l;k++) 
                printf("%.0f ", a[i][j][k]);
            printf("      \t");
        }
        printf("\n");
    }
    printf("\n");
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
