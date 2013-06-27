/* test1d.c - test 1 dimensional arrays from amalloc */

#include <stdio.h>
#include <assert.h>
#include "amalloc.h"

/* Three different signatures of a print function for a 1d array: */

/* print_1 takes a pointer array, whose elements could be changed. */
void print_1(float* a, int n);

/* print_2 wants the constant array as a const pointer. */
void print_2(const float* a, int n);

/* print_3 takes the dynamic array a, which already contains its dimension */
void print_3(const float* a);


int main() 
{
    const int n = 9;

    float* a = amalloc(sizeof(float),1,n);
    int i;
    for (i=0;i<n;i++)
        a[i] = i+1;
    print_1(a, asize(a,0));
    print_2(a, asize(a,0));
    print_3(a);
    afree(a);

    float b[n];
    for (i=0;i<n;i++)
      b[i]=i+1;

    print_1(b, sizeof(b)/sizeof(*b)); 
    print_2(b, sizeof(b)/sizeof(*b)); 
    #define amake1(b) amake(sizeof(b[0]),1,sizeof(b)/sizeof(b[0])) 
    /* float* ba = amake1(b); */
    /* print_3(ba); */
    /* makefree(ba); */
    return 0;
}


/******************************************************/
void print_1(float* a, int n) 
{
    int i;
    for (i=0;i<n;i++) 
        printf("%.5f ", a[i]);
    printf("\n");
}
/******************************************************/
void print_2(const float* a, int n) 
{
    int i;
    for (i=0;i<n;i++) 
        printf("%.5f ", a[i]);
    printf("\n");
}
/******************************************************/
void print_3(const float* a) 
{
    int i;
    assert(aknown(a));
    const int n = asize(a,0);
    for (i=0;i<n;i++) 
        printf("%.5f ", a[i]);
    printf("\n");
}
/******************************************************/
