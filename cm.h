#ifndef CM_H_INCLUDED
#define CM_H_INCLUDED

#include "defs.h"
#include "buffer.h"
#include "pmodels.h"
#include "cm_hash.h"
#include "cm_array.h"
#include "cm_tolerant.h"

#define ARRAY_MODE            0
#define HASH_TABLE_MODE       1

#define MAX_ARRAY_MEMORY      2048    //Size in MBytes

typedef struct{
  uint32_t   ctx;           // Current depth of context template
  uint64_t   nPModels;      // Maximum number of probability models
  uint32_t   alphaDen;      // Denominator of alpha
  uint32_t   maxCount;      // Counters /= 2 if one counter >= maxCount
  uint64_t   multiplier;
  uint8_t    ref;
  uint32_t   mode;
  HASH       *HT;
  ARRAY      *AT;
  TOLERANT   *TM;
  double     gamma;
  double     eGamma;
  uint64_t   pModelIdx;     // IDX
  uint64_t   pModelIdxIR;   // IDX INVERSIONS
  uint32_t   edits;         // EDITS
  uint32_t   nSym;          // EDITS
  uint32_t   ir;            // INVERTED REPEATS
  }
CMODEL;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void            GetPModelIdx         (uint8_t *, CMODEL *);
uint8_t         GetPModelIdxIR       (uint8_t *, CMODEL *);
uint64_t        GetPModelIdxCorr     (uint8_t *, CMODEL *, uint64_t);
void            ResetCModelIdx       (CMODEL *);
void            UpdateCModelCounter  (CMODEL *, uint32_t, uint64_t);
CMODEL          *CreateCModel        (uint32_t, uint32_t, uint8_t, uint32_t, 
                                      uint32_t, uint32_t, double, double,
                                      uint32_t, uint32_t);
void            ComputePModel        (CMODEL *, PMODEL *, uint64_t, uint32_t);
void            RemoveCModel         (CMODEL *);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
