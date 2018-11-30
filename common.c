#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "defs.h"
#include "common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Pow function from http://martin.ankerl.com/2007/10/04/
// optimized-pow-approximation-for-java-and-c-c/

double Power (double base, double exponent)
  {
  union
    {
    double d;
    int x[2];
    } u = { base };
  u.x[1] = (int) (exponent * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
  }


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

double Log(double base, double x)
  {
  return log(x) / log(base); 
  } 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CALCULATION OF CONTEXT MULTIPLICATOR FOR INDEX FUNCTION USAGE
//

uint64_t CalcMult(uint32_t c)
  {
  uint32_t n;
  uint64_t x[c], p = 1;
  for(n = 0 ; n < c ; ++n){
    x[n] = p;
    p  <<= 2;
    }
  return x[c-1];
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Progress(uint64_t size, uint64_t i)
  {
  if(size < 101) return;
  if(i % (size / 100) == 0 && size > PROGRESS_MIN)
    fprintf(stderr, "Progress:%3d %%\r", (uint8_t) (i / (size / 100)));
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
