#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "repeats.h"
#include "common.h"
#include "dna.h"
#include "mem.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// SHIFT BUFFER TO RIGHT. PERHAPS A CIRCULAR BUFFER IS A BETTER APPROACH...
//
void ShiftRBuf(uint8_t *b, uint32_t s, uint8_t n){
  memmove(b, b+1, s*sizeof(uint8_t));
  b[s-1] = n;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GET NUMERICAL BASE FROM PACKED SEQUENCE BY ID. THE ID%4 IS GIVEN BY THE 2
// LESS SIGNIFICATIVE BITS (ID&3).
//
uint8_t GetNBase(uint8_t *b, uint64_t i){
  return (uint8_t) (((0x3<<((3-(i&0x3))<<1)) & b[i>>2])>>((3-(i&0x3))<<1));
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CREATES THE RCLASS BASIC STRUCTURE 
//
RCLASS *CreateRC(uint32_t m, double a, double b, uint32_t l, uint32_t c,
double g, uint8_t i){
  RCLASS *C     = (RCLASS   *) Calloc(1,     sizeof(RCLASS  ));
  C->hash       = (RHASH    *) Calloc(1,     sizeof(RHASH   ));
  C->P          = (RPARAM   *) Calloc(1,     sizeof(RPARAM  ));
  C->hash->ent  = (RENTRY  **) Calloc(HSIZE, sizeof(RENTRY *));
  C->hash->size = (uint32_t *) Calloc(HSIZE, sizeof(uint32_t));
  C->RM         = (RMODEL   *) Calloc(m,     sizeof(RMODEL  ));
  C->mRM        = m;
  C->P->rev     = i;
  C->P->alpha   = ((int)(a*65534))/65534.0;
  C->P->beta    = ((int)(b*65534))/65534.0;
  C->P->gamma   = ((int)(g*65534))/65534.0;
  C->P->limit   = l;
  C->P->ctx     = c;
  C->P->mult    = CalcMult(c);
  C->P->idx     = 0;
  C->P->idxRev  = 0;
  return C;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// REVERSE COMPLEMENT INDEX BASED ON PAST SYMBOLS
//
uint64_t GetIdxRev(uint8_t *p, RCLASS *C){
  return (C->P->idxRev = (C->P->idxRev>>2)+CompNum(*p)*C->P->mult);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// INDEX CALC BASED ON PAST SYMBOLS
//
uint64_t GetIdx(uint8_t *p, RCLASS *C){
  return (C->P->idx = ((C->P->idx-*(p-C->P->ctx)*C->P->mult)<<2)+*p);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GET REPEAT MODEL RHASH RENTRY
//
RENTRY *GetHEnt(RCLASS *C, uint64_t key){

  uint32_t n, h = (uint32_t) (key % HSIZE);
  uint64_t b = (uint64_t) key & 0xfffffff0000;

  for(n = 0 ; n < C->hash->size[h] ; ++n)
    if(((uint64_t) C->hash->ent[h][n].key | b) == key)
      return &C->hash->ent[h][n];

  return NULL;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// START EACH REPEAT MODEL, CURRENTLY BY RANDOM STORED POSITION 
//
int32_t StartRM(RCLASS *C, uint32_t m, uint64_t i, uint8_t r){
  uint32_t s;
  RENTRY *E;

  if((E = GetHEnt(C, i)) == NULL)
    return 0;

  uint32_t idx;
  if(E->nPos > 0)
    idx = rand() % E->nPos;
  else
    idx = 0;

  if(r == 0) // REGULAR REPEAT
    #ifdef REPEATS_RANDOM
    C->RM[m].pos = E->pos[ idx ];
    #else
    C->RM[m].pos = E->pos[ 0 ];
    #endif
  else{ // INVERTED REPEAT
    #ifdef REPEATS_RANDOM
    if(E->pos[ idx ] <= C->P->ctx+1) 
      return 0;
    C->RM[m].pos = E->pos[ idx ] - C->P->ctx - 1;
    #else
    if(E->pos[ 0 ] <= C->P->ctx+1) return 0;
    C->RM[m].pos = E->pos[ 0 ] - C->P->ctx - 1;
    #endif
    }

  C->RM[m].nHits  = 0;
  C->RM[m].nTries = 0;
  C->RM[m].rev    = r;
  C->RM[m].acting = 0;
  C->RM[m].weight = INIWEIGHT;
  for(s = 0 ; s < NSYM ; ++s)
    C->RM[m].probs[s] = 0;

  return 1;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// INSERT KMER POSITION INTO RHASH TABLE 
//
void InsertKmerPos(RCLASS *C, uint64_t key, uint32_t pos){

  uint32_t n, h = (uint32_t) key % HSIZE;
  uint64_t b = key & 0xfffffff0000;
 
  for(n = 0 ; n < C->hash->size[h] ; ++n)
    if(((uint64_t) C->hash->ent[h][n].key | b) == key){
      C->hash->ent[h][n].pos = (uint32_t *) Realloc(C->hash->ent[h][n].pos, 
      (C->hash->ent[h][n].nPos + 1) * sizeof(uint32_t));
      C->hash->ent[h][n].pos[C->hash->ent[h][n].nPos++] = pos; 
      // STORE THE LAST K-MER POSITION
      return;
      }

  // CREATE A NEW RENTRY
  C->hash->ent[h] = (RENTRY *) Realloc(C->hash->ent[h],  
                    (C->hash->size[h]+1) * sizeof(RENTRY));
  
  // CREATE A NEW POSITION
  C->hash->ent[h][C->hash->size[h]].pos    = (uint32_t *) Calloc(1, 
                                             sizeof(uint32_t));
  C->hash->ent[h][C->hash->size[h]].nPos   = 1;
  C->hash->ent[h][C->hash->size[h]].pos[0] = pos;
  C->hash->ent[h][C->hash->size[h]].key    = (uint16_t) (key & 0xffff);
  C->hash->size[h]++;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMPUTE REPEAT MODEL PROBABILITIES
//
void ComputeRMProbs(RCLASS *C, RMODEL *R, uint8_t *b){
  uint8_t n, s;
  s = (R->rev == 1) ? CompNum(GetNBase(b, R->pos)) : GetNBase(b, R->pos);
  R->probs[s] = (R->nHits+C->P->alpha) / (R->nTries+2*C->P->alpha);
  for(n = 0 ; n < NSYM ; ++n)
    if(n != s){
      R->probs[n] = (1-R->probs[s])/3;
      }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// UPDATE REPEAT MODEL
//
void UpdateRM(RMODEL *R, uint8_t *b, uint8_t s){
  R->lastHit = 1;
  if(R->rev == 0){
    if(GetNBase(b, R->pos++) == s){
      R->nHits++;
      R->lastHit = 0;
      }
    }
  else{
    if(CompNum(GetNBase(b, R->pos--)) == s){
      R->nHits++;
      R->lastHit = 0;
      }
    }
  R->nTries++;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// RENORMALIZE REPEAT WEIGHTS
//
void RenormWeights(RCLASS *C){
  uint32_t n;
  double   t = 0;
  for(n = 0 ; n < C->nRM ; ++n)
    t += C->RM[n].weight;
  for(n = 0 ; n < C->nRM ; ++n)
    C->RM[n].weight /= t;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// STOP USELESS REPEAT MODELS
//
void StopRM(RCLASS *C){
  uint32_t n, a;
  do{
    a = 0;
    for(n = 0 ; n < C->nRM ; ++n)
      if((C->RM[n].acting = C->P->beta * C->RM[n].acting + C->RM[n].lastHit) > 
      C->P->limit * 1.0 || C->RM[n].pos == 0)
        {
        if(n != C->nRM-1)
          C->RM[n] = C->RM[C->nRM-1];
        C->nRM--;
        a = 1;
        break;
        }
    }
  while(a);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// START NEW REPEAT MODELS IF THERE IS STILL SPACE
//                         
void StartMultipleRMs(RCLASS *C, uint8_t *b){
  if(C->nRM < C->mRM && StartRM(C, C->nRM, GetIdx(b, C), 0))
    C->nRM++;

  if(C->P->rev == 1 && C->nRM < C->mRM && StartRM(C, C->nRM, 
  GetIdxRev(b, C), 1))
    C->nRM++;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMPUTE AND EXTRACT MIXTURED PROBABILITIES
//
void ComputeMixture(RCLASS *C, PMODEL *M, uint8_t *b){
  uint32_t r, s; 
  double F[NSYM] = {0,0,0,0};
  
  for(r = 0 ; r < C->nRM ; ++r){
    ComputeRMProbs(C, &C->RM[r], b);
    for(s = 0 ; s < NSYM ; ++s)
      F[s] += C->RM[r].probs[s] * C->RM[r].weight;
    }

  M->sum = 0;
  for(s = 0 ; s < NSYM ; ++s){
    M->freqs[s] = 1 + (uint32_t)(F[s] * MAXC);
    M->sum += M->freqs[s];
    }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// UPDATE WEIGHTS ACCORDING TO THE PERFORMANCE OF EACH REPEAT MODEL
//
void UpdateWeights(RCLASS *C, uint8_t *b, uint8_t s){
  uint32_t r;
  for(r = 0 ; r < C->nRM ; ++r){
    C->RM[r].weight = Power(C->RM[r].weight, C->P->gamma) * C->RM[r].probs[s];
    UpdateRM(&C->RM[r], b, s);
    }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
