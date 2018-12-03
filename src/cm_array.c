#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "mem.h"
#include "cm_array.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ARRAY *CreateArrayTable(uint64_t nSym, uint64_t length){ 
  ARRAY *A    = (ARRAY *) Calloc(1, sizeof(ARRAY));
  A->nSym     = nSym;
  A->length   = length;
  A->counters = (ACC *) Calloc(A->length * A->nSym, sizeof(ACC));
  return A;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UpdateArrayCounter(ARRAY *A, uint32_t sym, uint64_t idx){
  uint32_t x;
  ACC *AC = &A->counters[idx * A->nSym];
  if(++AC[sym] == MAX_ARRAY_COUNTER){    
    for(x = 0 ; x < A->nSym ; ++x)
      AC[x] /= 2;
    }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void RemoveArrayTable(ARRAY *A){
  Free(A->counters);
  Free(A);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

