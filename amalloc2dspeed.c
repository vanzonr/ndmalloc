/* 
 * amalloc2dspeed - speed test for amalloc dynamic array library
 */
//

#include "cstopwatch.h"
#include "amalloc.h"
#include <stdlib.h>
#include <math.h>

  const int repeat = 2;
/*const int n = 13376; */ /* requires ~2GB of storage*/
/*const int n = 9458;  */ /* requires ~1GB of storage */
const int n = 8458;
double case_exact(int repeat)
{
    double check = repeat*(n-1);
    if (2*(repeat/2)==repeat) 
        check += (repeat/2)*(3*repeat/2-2);       
    else
        check += (repeat-1)*(3*repeat-1)/4;    
    return n*n*check;
}

extern void pass(float*,float*,int*); // calling this function between
                                      // loops prevents loop fusion
                                      // and unfair speed gains with
                                      // automatically arrays where
                                      // the intel compiler
                                      // optimizes much of the
                                      // computations away!




































































double case_amalloc(int repeat)
{
    int i, j;
    double d = 0.0;
    float** a = amalloc(sizeof(float), 2, n, n);
    float** b = amalloc(sizeof(float), 2, n, n);
    float** c = amalloc(sizeof(float), 2, n, n);





    while (repeat--) {
        for (i=0;i<n;i++)
          //#pragma parallel
            for (j=0;j<n;j++) {
               a[i][j] = i+repeat;
               b[i][j] = j+repeat/2;
            }
        pass(&a[0][0],&b[0][0],&repeat);
        for (i=0;i<n;i++)
          //#pragma parallel
            for (j=0;j<n;j++) 
                c[i][j] = a[i][j]+b[i][j];
        pass(&c[0][0],&c[0][0],&repeat);
        for (i=0;i<n;i++)
          //            #pragma simd
            for (j=0;j<n;j++) 
                d += c[i][j];
        pass(&c[0][0],(float*)&d,&repeat);
    }





    afree(a);
    afree(b);
    afree(c);
    return d;
}



























































double case_dyn(int repeat)
{
    int i, j;
    double d = 0.0;
    float** a = malloc(sizeof(float*)*n);
    float** b = malloc(sizeof(float*)*n);
    float** c = malloc(sizeof(float*)*n);
    for (i=0;i<n;i++) {
        a[i] = malloc(sizeof(float)*n);
        b[i] = malloc(sizeof(float)*n);
        c[i] = malloc(sizeof(float)*n);
    }
    while (repeat--) {
        for (i=0;i<n;i++)
          //            #pragma parallel
            for (j=0;j<n;j++) {
                a[i][j] = i+repeat;
                b[i][j] = j+repeat/2;
            }
        pass(a[0],b[0],&repeat);
        for (i=0;i<n;i++)
          //#pragma parallel
            for (j=0;j<n;j++) 
                c[i][j] = a[i][j]+b[i][j];
        pass(c[0],c[0],&repeat);
        for (i=0;i<n;i++)
          // #pragma simd
            for (j=0;j<n;j++) 
                d += c[i][j];
        pass(c[0],(float*)&d,&repeat);
    }
    for (i=0;i<n;i++) {
        free(a[i]);
        free(b[i]);
        free(c[i]);
    }
    free(a);
    free(b);
    free(c);
    return d;
}

double case_auto(int repeat) 
{
    int i, j;
    double d = 0.0;
    float a[n][n], b[n][n], c[n][n];
    while (repeat--) {
        for ( i=0;i<n;i++)
            for (j=0;j<n;j++) {
                a[i][j] = i+repeat;
                b[i][j] = j+repeat/2;
            }
        pass(a[0],b[0],&repeat);
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) 
                c[i][j] = a[i][j]+b[i][j];
        pass(c[0],c[0],&repeat);
        for (i=0;i<n;i++)
            for (j=0;j<n;j++) 
                 d += c[i][j];
        pass(c[0],(float*)&d,&repeat);
    }
    return d;
}

int main(int argc, char**argv) 
{
    Stopwatch s;
    double answer;
    int thiscase = (argc==1)?1:atoi(argv[1]);

#define STOPWATCH(x,r) x(1); stopwatchStart(&s); answer += x(r-1); s.prefix='\t'; stopwatchStop(&s);

    if (argc>2)
       printf("%s\t",argv[2]);

    switch (thiscase) {
    case 0: 
        printf("exact");
        fflush(stdout);
        answer = STOPWATCH(case_exact,repeat);
        break;
    case 1: 
        printf("amalloc");
        fflush(stdout);
        answer = STOPWATCH(case_amalloc, repeat);
        break;
    case 2: 
         printf("automatic");
         fflush(stdout);
         answer = STOPWATCH(case_auto, repeat);
         break;
    case 3: 
        printf("dynamic");
        fflush(stdout);
        answer = STOPWATCH(case_dyn, repeat);
        break;
    }

    double check = case_exact(1)+case_exact(repeat-1);
    double eps = 1e-6;

    if (fabs(1-answer/check)>eps)
        printf("%lf does not match exact result of %lf\n", 
               answer/n/n, check/n/n);
    // else
    //     printf("%lf matches exact result!\n", answer/n/n);
    return 0;
}

