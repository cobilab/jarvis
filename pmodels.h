#ifndef PMODELS_H_INCLUDED
#define PMODELS_H_INCLUDED

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct{
  uint32_t *freqs;
  uint32_t sum;
  }
PMODEL;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

PMODEL *CreatePM   (uint32_t);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
