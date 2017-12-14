#include <stdio.h>
#include <stdlib.h>
#include "pmodels.h"
#include "mem.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

PMODEL *CreatePM(uint32_t n){
  PMODEL *P = (PMODEL   *) Calloc(1, sizeof(PMODEL));
  P->freqs  = (uint32_t *) Calloc(n, sizeof(uint32_t));
  return P;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
