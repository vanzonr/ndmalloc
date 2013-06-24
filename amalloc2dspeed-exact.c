/* 
 * amalloc2dspeed-exact.c - comparison case for array speed tests
 */

#include <stdlib.h>
#include "ndef.h"

double case_exact(int repeat)
{
    double check = repeat*(n-1);
    if (2*(repeat/2)==repeat) 
        check += (repeat/2)*(3*repeat/2-2);       
    else
        check += (repeat-1)*(3*repeat-1)/4;    
    return n*n*check;
}

