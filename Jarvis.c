//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                           J A R V I S   2 0 1 4                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <time.h>
#include <malloc.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <unistd.h>
#include "bitio.h"
#include "arith.h"
#include "arith_aux.h"

#define MAX_BUF    1000000
#define SCACHE     32
#define NSYM       4
#define MAXC       65535 //((1<<(sizeof(uint16_t)*8))-1)

#define DEF_MRM    50
#define DEF_CTX    11
#define DEF_ALPHA  1
#define DEF_GAMMA  0.95
#define DEF_BETA   0.90
#define DEF_LIMIT  5
#define DEF_REV    0
#define DEF_MODE   0
#define INIWEIGHT  0.999

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// SIMPLE MEMORY HANDLING FUNCTIONS
//
void ErrorQuit(size_t s){
  fprintf(stderr, "Error allocating %zu bytes.\n", s);
  exit(1);
  }

void *Malloc(size_t s){
  void *p = malloc(s);
  if(p == NULL) ErrorQuit(s);
  return p;
  }

void *Calloc(size_t n, size_t s){
  void *p = calloc(n, s);
  if(p == NULL) ErrorQuit(s);
  return p;
  }

void *Realloc(void *r, size_t s){
  void *p = realloc(r, s);
  if(p == NULL) ErrorQuit(s);
  return p;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// SIMPLE STRINGS CONCATENATION FUNCTION 
//
char *Cat(char *a, char *b){
  char *c = (char *) Malloc(strlen(a)+strlen(b)+1);
  strcpy(c, a);
  strcat(c, b);
  return c;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// SIMPLE FILE HANDLING FUNCTIONS
//
FILE *Fopen(const char *p, const char *m){
  FILE *F = fopen(p, m);
  if(F == NULL){
    fprintf(stderr, "Error opening: %s (mode %s). Does it exist?\n", p, m);
    exit(1);
    }
  return F;
  }

uint64_t FNBytes(FILE *F)
  {
  uint64_t n = 0;
  fseek(F, 0, SEEK_END);
  n = ftell(F);
  rewind(F);
  return n;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// PERCENTAGE OF SEQUENCE COMPUTATION PROGRESS 
//
void Progress(uint64_t s, uint64_t i){
  if(i%(s/100) == 0 && s > 100)
    fprintf(stderr, "Progress:%3d %%\r", (uint8_t) (i/(s/100)));
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// SHIFT BUFFER TO RIGHT. PERHAPS A CIRCULAR BUFFER IS A BETTER APPROACH...
//
void ShiftRBuf(uint8_t *b, uint32_t s, uint8_t n){
  memmove(b, b+1, s*sizeof(uint8_t));
  b[s-1] = n;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// APPROXIMATED POWER FUNCTION [A LOT FASTER] FROM: http://martin.ankerl.com/
// 2007/10/04/optimized-pow-approximation-for-java-and-c-c/
// 
double PW(double a, double b){
  int t = (*(1+(int*)&a)), t2 = (int)(b*(t-1072632447)+1072632447);
  double p = 0.0;
  *(1+(int*)&p) = t2;
  return p;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// SIMPLE ARGUMENTS PARSE FUNCTIONS
//
uint32_t ArgNum(uint32_t d, char *a[], uint32_t n, char *s){
  for( ; --n ; ) if(!strcmp(s, a[n])) return atol(a[n+1]);
  return d;
  }

double ArgDbl(double d, char *a[], uint32_t n, char *s){
  for( ; --n ; ) if(!strcmp(s, a[n])) return atof(a[n+1]);
  return d;
  }

uint8_t ArgBin(uint8_t d, char *a[], uint32_t n, char *s){
  for( ; --n ; ) if(!strcmp(s, a[n])) return d==0?1:0;
  return d;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CONVERSION FROM NUMERICAL {0,1,2,3} TO SYMBOLICAL DNA {A,C,G,T}
//
uint8_t N2S(uint8_t x){
  switch(x){
    case 0: return 'A'; case 1: return 'C';
    case 2: return 'G'; case 3: return 'T';
    }
  printf("Error: unknown numercial symbol!\n");
  exit(1);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CONVERSION FROM SYMBOLICAL DNA {A,C,G,T} TO NUMERICAL{0,1,2,3}
//
uint8_t S2N(uint8_t x){
  switch(x){
    case 'A': return 0; case 'C': return 1;
    case 'G': return 2; case 'T': return 3;
    }
  printf("Error: found ascii index: %d. Alphabet is limited to {ACGT}!\n", x);
  exit(1);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// NUMERICAL COMPLEMENT CONVERSION {0,1,2,3} -> {3,2,1,0} (SAME AS: 3-BASE)
//
uint8_t Comp(uint8_t x){
  switch(x){
    case 0: return 3; case 1: return 2; case 2: return 1; case 3: return 0;
    }
  printf("Error: invalid complement alphabet source!\n");
  exit(1);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CALCULATION OF CONTEXT MULTIPLICATOR FOR INDEX FUNCTION USAGE
//
uint64_t CalcMult(uint32_t c){
  uint32_t n;
  uint64_t x[c], p = 1;
  for(n = 0 ; n < c ; ++n){
    x[n] = p;
    p  <<= 2;
    }
  return x[c-1];
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GET NUMERICAL BASE FROM PACKED SEQUENCE BY ID. THE ID%4 IS GIVEN BY THE 2
// LESS SIGNIFICATIVE BITS (ID&3).
//
uint8_t GetNBase(uint8_t *b, uint64_t i){
  return (uint8_t) (((0x3<<((3-(i&0x3))<<1)) & b[i>>2])>>((3-(i&0x3))<<1));
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// MODEL AND FUNCTION TO STORE PROBABILITIES
//
typedef struct{
  uint32_t *freqs;
  uint32_t sum;
  }
PMODEL;

PMODEL *CreatePM(uint32_t n){
  PMODEL *P = (PMODEL   *) Calloc(1, sizeof(PMODEL));
  P->freqs  = (uint32_t *) Calloc(n, sizeof(uint32_t));
  return P;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// HASH TABLE TO STORE REPEATING POSITIONS AND INDEXES ALONG THE SEQUENCE
// DO NOT CHANGE THESE MACRO VALUES UNLESS YOU REALLY KNOW WHAT YOU ARE DOING!
//
#define HSIZE        16777259 // NEXT PRIME AFTER 16777216 (24 BITS)
#define MAX_CTX      20       // ((HASH_SIZE (24 B) + KEY (16 B))>>1) = 20 

typedef struct{
  uint16_t key;      // THE KEY (INDEX / HASHSIZE) STORED IN THIS ENTRY
  uint32_t pos;      // THE LAST (NEAREST) REPEATING POSITION
  }
ENTRY;

typedef struct{
  uint16_t *size;    // NUMBER OF KEYS FOR EACH ENTRY
  ENTRY    **ent;    // ENTRIES VECTORS POINTERS
  }
HASH;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// REPEAT MODELS TO HANDLE LONG SEGMENTS. DATA SUBSTITUTIONS DO NOT AFFECT THE
// PERFORMANCE SO MUCH AS IN CONTEXT MODELS.
//
typedef struct{
  uint64_t idx;      // CURRENT CONTEXT INDEX
  uint64_t idxRev;   // CURRENT INVERTED REPEAT INDEX
  uint64_t mult;     // CURRENT INVERTED REPEAT INDEX
  uint32_t ctx;      // CONTEXT TEMPLATE SIZE FOR REPEAT MODEL
  uint32_t limit;    // REPEAT PERFORMANCE LIMIT, ASSOCIATED WITH BETA
  double   alpha;    // ALPHA PROBABILITY ESTIMATOR
  double   beta;     // REPEAT PERFORMANCE DECAYING FOR REPEAT MOVE
  double   gamma;    // PERFORMANCE DECAYING PARAMETER
  uint8_t  rev;      // INVERTED REPEAT USAGE [MEMORY/TIME PAINFUL]
  }
PARAM;

typedef struct{
  uint32_t pos;      // XXX: Position of the next predicted symbol ??
  uint32_t nHits;    // NUMBER OF TIMES THIS MODEL WAS CORRECT
  uint32_t nTries;   // NUMBER OF TIMES THIS MODEL WAS USED
  double   probs[4]; // REPEAT MODEL SYMBOL PROBABILITIES
  double   weight;   // WEIGHT OF THE MODEL FOR MIXTURE
  double   acting;   // XXX: WTF? 
  double   lastHit;  // XXX: WTF? 
  uint8_t  rev;      // INVERTED REPETAT MODEL. IF REV='Y' THEN IS TRUE
  }
RMODEL;

typedef struct{
  HASH     *hash;    // REPEATING KMERS HASH TABLE
  RMODEL   *RM;      // POINTER FOR EACH OF THE MULTIPLE REPEAT MODELS
  PARAM    *P;       // EXTRA PARAMETERS FOR REPEAT MODELS
  uint32_t nRM;      // CURRENT NUMBER OF REPEAT MODELS
  uint32_t mRM;      // MAXIMUM NUMBER OF REPEAT MODELS
  uint64_t size;     // SIZE OF THE INPUT SEQUENCE
  }
RCLASS;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CREATES THE RCLASS BASIC STRUCTURE 
//
RCLASS *CreateRC(uint32_t m, double a, double b, uint32_t l, uint32_t c,
double g, uint8_t i){
  RCLASS *C     = (RCLASS   *) Calloc(1,     sizeof(RCLASS  ));
  C->hash       = (HASH     *) Calloc(1,     sizeof(HASH    ));
  C->P          = (PARAM    *) Calloc(1,     sizeof(PARAM   ));
  C->hash->ent  = (ENTRY   **) Calloc(HSIZE, sizeof(ENTRY  *));
  C->hash->size = (uint16_t *) Calloc(HSIZE, sizeof(uint16_t));
  C->RM         = (RMODEL   *) Calloc(m,     sizeof(RMODEL  ));
  C->mRM        = m;
  C->P->rev     = i;
  C->P->alpha   = ((int)(a*65535))/65535.0;
  C->P->beta    = ((int)(b*65535))/65535.0;
  C->P->gamma   = ((int)(g*65535))/65535.0;
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
  return (C->P->idxRev = (C->P->idxRev>>2)+Comp(*p)*C->P->mult);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// INDEX CALC BASED ON PAST SYMBOLS
//
uint64_t GetIdx(uint8_t *p, RCLASS *C){
  return (C->P->idx = ((C->P->idx-*(p-C->P->ctx)*C->P->mult)<<2)+*p);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GET REPEAT MODEL HASH ENTRY
//
ENTRY *GetHEnt(RCLASS *C, uint64_t key){
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
  ENTRY *E;

  if((E = GetHEnt(C, i)) == NULL)
    return 0;

  if(r == 0)
    C->RM[m].pos = E->pos;
  else{
    if(E->pos <= C->P->ctx+1) return 0;
    C->RM[m].pos = E->pos-C->P->ctx-1;
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
// INSERT KMER POSITION INTO HASH TABLE 
//
void InsertKmerPos(RCLASS *C, uint64_t key, uint32_t pos){
  uint32_t n, h = (uint32_t) key % HSIZE;
  uint64_t b = key & 0xfffffff0000;
 
  for(n = 0 ; n < C->hash->size[h] ; ++n)
    if(((uint64_t) C->hash->ent[h][n].key | b) == key){
      C->hash->ent[h][n].pos = pos;           // STORE THE LAST K-MER POSITION
      return;
      }

  // CREATE A NEW ENTRY
  C->hash->ent[h] = (ENTRY *) Realloc(C->hash->ent[h], (C->hash->size[h]+1) * 
  sizeof(ENTRY));

  // CREATE A NEW POSITION
  C->hash->ent[h][C->hash->size[h]].pos = pos;
  C->hash->ent[h][C->hash->size[h]].key = (uint16_t) (key & 0xffff);
  C->hash->size[h]++;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMPUTE REPEAT MODEL PROBABILITIES
//
void ComputeRMProbs(RCLASS *C, RMODEL *R, uint8_t *b){
  uint8_t n, s;
  s = (R->rev == 1) ? Comp(GetNBase(b, R->pos)) : GetNBase(b, R->pos);
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
    if(Comp(GetNBase(b, R->pos--)) == s){
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
  uint32_t n;
  uint8_t  a;
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
// COMPUTE AND EXTRAC MIXTURED PROBABILITIES
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
    C->RM[r].weight = PW(C->RM[r].weight, C->P->gamma) * C->RM[r].probs[s];
    UpdateRM(&C->RM[r], b, s);
    }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// DECODE HEADER AN CREATE REPEAT CLASS
//
RCLASS *DecodeHeader(FILE *F){
  uint64_t s;
  uint32_t m, l, c;
  double   a, b, g;
  uint8_t  r;
  RCLASS   *C;
  s = ReadNBits(64, F);
  m = ReadNBits(32, F);
  a = ReadNBits(16, F) / 65535.0;
  b = ReadNBits(16, F) / 65535.0;
  g = ReadNBits(16, F) / 65535.0;
  l = ReadNBits(16, F);
  c = ReadNBits(16, F);
  r = ReadNBits( 1, F);
  C = CreateRC(m, a, b, l, c, g, r);
  C->size = s;

  #ifdef DEBUG
  printf("size = %"PRIu64"\n", C->size);
  printf("max rep = %u\n",     C->mRM);
  printf("alpha = %g\n",       C->P->alpha);
  printf("beta = %g\n",        C->P->beta);
  printf("gamma = %g\n",       C->P->gamma);
  printf("limit = %u\n",       C->P->limit);
  printf("ctx = %u\n",         C->P->ctx);
  printf("ir = %u\n",          C->P->rev);
  #endif

  return C;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ENCODE HEADER
//
void EncodeHeader(RCLASS *C, FILE *F){
  WriteNBits(C->size,                       64, F);
  WriteNBits(C->mRM,                        32, F);
  WriteNBits((uint16_t)(C->P->alpha*65535), 16, F);
  WriteNBits((uint16_t)(C->P->beta *65535), 16, F);
  WriteNBits((uint16_t)(C->P->gamma*65535), 16, F);
  WriteNBits(C->P->limit,                   16, F);
  WriteNBits(C->P->ctx,                     16, F);
  WriteNBits(C->P->rev,                      1, F);
  
  #ifdef DEBUG
  printf("size = %"PRIu64"\n", C->size);
  printf("max rep = %u\n",     C->mRM);
  printf("alpha = %g\n",       C->P->alpha);
  printf("beta = %g\n",        C->P->beta);
  printf("gamma = %g\n",       C->P->gamma);
  printf("limit = %u\n",       C->P->limit);
  printf("ctx = %u\n",         C->P->ctx);
  printf("ir = %u\n",          C->P->rev);
  #endif
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMPRESSION
//
void Compress(RCLASS *C, char *fn){
  FILE     *IN  = Fopen(fn, "r"), *OUT = Fopen(Cat(fn, ".co"), "w");
  uint64_t i = 0, mSize = MAX_BUF, pos = 0;
  uint32_t m, n; 
  uint8_t  t[NSYM], *buf = (uint8_t *) Calloc(mSize, sizeof(uint8_t)), 
           *cache = (uint8_t *) Calloc(SCACHE+1, sizeof(uint8_t)), sym = 0;
  PMODEL   *MX = CreatePM(NSYM);

  C->size = FNBytes(IN)>>2;

  startoutputtingbits();
  start_encode();
  EncodeHeader(C, OUT);

  while((m = fread(t, sizeof(uint8_t), NSYM, IN)) == NSYM){
    buf[i] = S2N(t[3])|(S2N(t[2])<<2)|(S2N(t[1])<<4)|(S2N(t[0])<<6); // PACK 4
    
    for(n = 0 ; n < m ; ++n){
      sym = S2N(t[n]);
      StopRM(C);
      StartMultipleRMs(C, cache+SCACHE-1);
      InsertKmerPos(C, C->P->idx, pos++);                    // pos = (i<<2)+n
      RenormWeights(C);
      ComputeMixture(C, MX, buf);
      ArithEncodeSymbol(sym, (int *)(MX->freqs), (int) MX->sum, OUT);
      UpdateWeights(C, buf, sym);
      ShiftRBuf(cache, SCACHE, sym);  // STORE THE LAST SCACHE BASES & SHIFT 1
      }

    if(++i == mSize)    // REALLOC BUFFER ON OVERFLOW 4 STORE THE COMPLETE SEQ
      buf = (uint8_t *) Realloc(buf, (mSize<<=1) * sizeof(uint8_t));

    Progress(C->size, i); 
    }

  printf("Size of DNA sequence: %"PRIu64"\n", pos);

  WriteNBits(m&3, 2, OUT);
  while(m--) WriteNBits(S2N(t[m]), 2, OUT);        // ENCODE REMAINING SYMBOLS

  finish_encode(OUT);
  doneoutputtingbits(OUT);
  fclose(IN);
  fclose(OUT);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// DECOMPRESSION
//
void Decompress(char *fn){
  FILE     *IN  = Fopen(fn, "r"), *OUT = Fopen(Cat(fn, ".jd"), "w");
  uint64_t i = 0, mSize = MAX_BUF, pos = 0;
  uint32_t m, n;
  uint8_t  *buf = (uint8_t *) Calloc(mSize, sizeof(uint8_t)),
           *cache = (uint8_t *) Calloc(SCACHE+1, sizeof(uint8_t)), sym = 0;
  PMODEL   *MX = CreatePM(NSYM);

  startinputtingbits();
  start_decode(IN);
  RCLASS *C = DecodeHeader(IN);

  while(i < C->size){                         // NOT absolute size (CHAR SIZE)
    for(n = 0 ; n < NSYM ; ++n){
      StopRM(C);
      StartMultipleRMs(C, cache+SCACHE-1);
      InsertKmerPos(C, C->P->idx, pos++);                    // pos = (i<<2)+n
      RenormWeights(C);
      ComputeMixture(C, MX, buf);
      sym = ArithDecodeSymbol(NSYM, (int *) MX->freqs, (int) MX->sum, IN);
      if(n == 0) buf[i] = sym<<6 ; else buf[i] |= (sym<<((3-n)<<1));
      fputc(N2S(sym), OUT);
      UpdateWeights(C, buf, sym);
      ShiftRBuf(cache, SCACHE, sym);  // STORE THE LAST SCACHE BASES & SHIFT 1
      }

    if(++i == mSize) // REALLOC BUFFER ON OVERFLOW 4 STORE THE COMPLETE SEQ
      buf = (uint8_t *) Realloc(buf, (mSize<<=1) * sizeof(uint8_t));

    Progress(C->size, i);
    }

  m = ReadNBits(2, IN);
  while(m--) fputc(N2S(ReadNBits(2, IN)), OUT);    // DECODE REMAINING SYMBOLS

  finish_decode();
  doneinputtingbits();
  fclose(IN);
  fclose(OUT);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// MAIN 
//
int main(int argc, char **argv){
  char     **p = *&argv, i, d;
  uint32_t c, l, m;
  double   a, g, b;
  RCLASS   *C;

  if(argc < 2){
    fprintf(stderr, "Usage: Jarvis [OPTIONS]... [FILE]              \n"); 
    fprintf(stderr, "                                               \n"); 
    fprintf(stderr, "  -m <REPEATS>  maximum number of repeats      \n"); 
    fprintf(stderr, "  -c <CONTEXT>  context order (k-mer)          \n"); 
    fprintf(stderr, "  -a <ALPHA>    alpha probabilities estimator  \n"); 
    fprintf(stderr, "  -b <BETA>     acceptance threshold           \n"); 
    fprintf(stderr, "  -l <LIMIT>    limit for acceptance threshold \n"); 
    fprintf(stderr, "  -g <GAMMA>    mixture decay factor           \n"); 
    fprintf(stderr, "  -i            use inverted repeats           \n"); 
    fprintf(stderr, "                                               \n"); 
    fprintf(stderr, "  -d            decompression mode             \n"); 
    fprintf(stderr, "                                               \n"); 
    fprintf(stderr, "  <FILE>        target file                    \n");
    return 0;
    }

  m = ArgNum(DEF_MRM,   p, argc, "-m");    // SEE THE HEAD OF THE FILE FOR THE 
  c = ArgNum(DEF_CTX,   p, argc, "-c");    // DEFAULT& NAIVE ESTIMATED VALUES.
  a = ArgDbl(DEF_ALPHA, p, argc, "-a");    // THE ESTIMATION OF THE PARAMETERS
  g = ArgDbl(DEF_GAMMA, p, argc, "-g");    // IMPROVE SUBSTANTIALLY THE REPEAT
  b = ArgDbl(DEF_BETA,  p, argc, "-b");    // BASED COMPRESSION. AS SUCH, SOME
  l = ArgNum(DEF_LIMIT, p, argc, "-l");    // TIME IN FUTURE WORKS MAY BE LOST
  i = ArgBin(DEF_REV,   p, argc, "-i");    // FOR ESTIMATION OR IN INTELLIGENT
  d = ArgBin(DEF_MODE,  p, argc, "-d");    // PREDICTION MODELLING DESIGN.
 
  if(!d){
    fprintf(stderr, "Compressing ...\n"); 
    C = CreateRC(m, a, b, l, c, g, i);
    Compress(C, argv[argc-1]);
    }
  else{
    fprintf(stderr, "Decompressing ...\n"); 
    Decompress(argv[argc-1]);
    }
 
  printf("Jarvis complete!");
  return 0;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
