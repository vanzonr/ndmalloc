/* 
 * amalloc2dspeed - speed test for amalloc dynamic array library
 */

#include <stdlib.h>
#include "test_damalloc.h"
#include "cstopwatch.h"
#include "amalloc.h"
#include "pass.h"
#include "ndef.h"

#include "amalloc2dspeed-exact.h"
#include "amalloc2dspeed-amalloc.h"
#include "amalloc2dspeed-dynamic.h"
#include "amalloc2dspeed-auto.h"

int main(int argc, char**argv) 
{
    Stopwatch s;
    double answer = 0.0;
    int thiscase  = (argc<=1)?1:atoi(argv[1]);
    int repeat    = (argc<=2)?9:atoi(argv[2]);

    printf("%dx ",repeat);

#define STOPWATCH(x,r) x(1); stopwatchStart(&s); answer += x(r-1); s.prefix='\t'; stopwatchStop(&s);

    if (argc>3)
       printf("%s\t",argv[3]);

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

    if ((1-answer/check)>eps ||
        (1-answer/check)<-eps)
        printf("%lf does not match exact result of %lf\n", 
               answer/n/n, check/n/n);
    // else
    //     printf("%lf matches exact result!\n", answer/n/n);
    return 0;
}

/********************************************************************/
