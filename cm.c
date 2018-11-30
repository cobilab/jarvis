#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "defs.h"
#include "dna.h"
#include "mem.h"
#include "common.h"
#include "cm.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CMODEL *CreateCModel(uint32_t ctx, uint32_t aDen, uint8_t ref, uint32_t edits, 
uint32_t eDen, uint32_t nSym, double gamma, double eGamma, uint32_t ir, 
uint32_t eIr){
  CMODEL    *M = (CMODEL *) Calloc(1, sizeof(CMODEL));
  uint64_t  prod = 1, *mult;
  uint32_t  n;

  M->nSym        = nSym;
  mult           = (uint64_t *) Calloc(ctx, sizeof(uint64_t));
  M->nPModels    = (uint64_t) pow(M->nSym, ctx);
  M->ctx         = ctx;
  M->ir          = ir;
  M->alphaDen    = aDen;
  M->edits       = edits;
  M->gamma       = gamma;
  M->eGamma      = eGamma;
  M->pModelIdx   = 0;
  M->pModelIdxIR = M->nPModels - 1;
  M->ref         = ref == 0 ? 0 : 1;

  if(M->nPModels * M->nSym * sizeof(ACC) >> 20 > MAX_ARRAY_MEMORY){
    M->mode = HASH_TABLE_MODE;
    M->HT   = CreateHashTable(M->nSym);
    }
  else{
    M->mode = ARRAY_MODE;
    M->AT   = CreateArrayTable(M->nSym, M->nPModels);
    }

  for(n = 0 ; n < M->ctx ; ++n){
    mult[n] = prod;
    prod *= M->nSym;
    }

  M->multiplier = mult[M->ctx-1];

  if(edits != 0)
    M->TM = CreateTolerantModel(edits, eDen, M->ctx, nSym, eIr);

  Free(mult);
  return M;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetCModelIdx(CMODEL *M){
  M->pModelIdx   = 0;
  M->pModelIdxIR = M->nPModels - 1;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void GetPModelIdx(uint8_t *p, CMODEL *M){
  M->pModelIdx = ((M->pModelIdx-*(p-M->ctx)*M->multiplier)*M->nSym)+*p;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t GetPModelIdxIR(uint8_t *p, CMODEL *M){
  M->pModelIdxIR = (M->pModelIdxIR>>2)+CompNum(*p)*M->multiplier;
  return CompNum(*(p-M->ctx));
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t GetPModelIdxCorr(uint8_t *p, CMODEL *M, uint64_t idx){
  return (((idx-*(p-M->ctx)*M->multiplier)*M->nSym)+*p);
  }
 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ComputePModel(CMODEL *M, PMODEL *PM, uint64_t idx, uint32_t aDen){
  ACC *ac;
  HCC *hc;
  uint32_t x;
  switch(M->mode){

    case HASH_TABLE_MODE:
      if(!(hc = GetHCCounters(M->HT, idx)))
        hc = (HCC *) M->HT->zeroCounters;
      PM->sum = 0;
      for(x = 0 ; x < M->nSym ; ++x){
        PM->freqs[x] = 1 + aDen * hc[x];
        PM->sum += PM->freqs[x];
        }
    break;

    case ARRAY_MODE:
      ac = &M->AT->counters[idx*M->nSym];
      PM->sum = 0;
      for(x = 0 ; x < M->nSym ; ++x){
        PM->freqs[x] = 1 + aDen * ac[x];
        PM->sum += PM->freqs[x];
        }
    break;

    default:
    fprintf(stderr, "Error: not implemented!\n");
    exit(1);
    }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UpdateCModelCounter(CMODEL *M, uint32_t sym, uint64_t idx){
  if(M->mode == HASH_TABLE_MODE) UpdateHashCounter (M->HT, sym, idx);
  else                           UpdateArrayCounter(M->AT, sym, idx);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void RemoveCModel(CMODEL *M){
  if(M->mode == HASH_TABLE_MODE) RemoveHashTable (M->HT);
  else                           RemoveArrayTable(M->AT);

  if(M->edits > 0)               RemoveTolerantModel(M->TM);
  Free(M);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

