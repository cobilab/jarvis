#ifndef REPEATS_H_INCLUDED
#define REPEATS_H_INCLUDED

#include "defs.h"
#include "pmodels.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define MAX_BUF          1000000
#define SCACHE           32
#define NSYM             4
#define MAXC             65535 //((1<<(sizeof(uint16_t)*8))-1)
#define REPEATS_RANDOM   1
#define DEF_MRM          50
#define DEF_CTX          16
#define DEF_ALPHA        1
#define DEF_GAMMA        0.2
#define DEF_BETA         0.90
#define DEF_LIMIT        5
#define DEF_REV          0
#define DEF_MODE         0
#define INIWEIGHT        0.999

#define HSIZE        16777259 // NEXT PRIME AFTER 16777216 (24 BITS)
#define MAX_CTX      20       // ((HASH_SIZE (24 B) + KEY (16 B))>>1) = 20 

typedef struct{
  uint16_t key;      // THE KEY (INDEX / HASHSIZE) STORED IN THIS RENTRY
  uint16_t nPos;     // NUMBER OF POSITIONS FOR THIS RENTRY
  uint32_t *pos;     // THE LAST (NEAREST) REPEATING POSITION
  }
RENTRY;

typedef struct{
  uint32_t *size;    // NUMBER OF KEYS FOR EACH RENTRY
  RENTRY   **ent;    // ENTRIES VECTORS POINTERS
  }
RHASH;

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
RPARAM;

typedef struct{
  uint32_t pos;      // POSITION OF THE SYMBOL
  uint32_t nHits;    // NUMBER OF TIMES THIS MODEL WAS CORRECT
  uint32_t nTries;   // NUMBER OF TIMES THIS MODEL WAS USED
  double   probs[4]; // REPEAT MODEL SYMBOL PROBABILITIES
  double   weight;   // WEIGHT OF THE MODEL FOR MIXTURE
  double   acting;   // THE ACTING PERFORMANCE
  double   lastHit;  // IS ON OR NOT
  uint32_t id;       // ID OF THE RHASH
  uint8_t  rev;      // INVERTED REPETAT MODEL. IF REV='Y' THEN IS TRUE
  }
RMODEL;

typedef struct{
  RHASH    *hash;    // REPEATING KMERS HASH TABLE
  RMODEL   *RM;      // POINTER FOR EACH OF THE MULTIPLE REPEAT MODELS
  RPARAM   *P;       // EXTRA PARAMETERS FOR REPEAT MODELS
  uint32_t nRM;      // CURRENT NUMBER OF REPEAT MODELS
  uint32_t mRM;      // MAXIMUM NUMBER OF REPEAT MODELS
  uint64_t size;     // SIZE OF THE INPUT SEQUENCE PACKED
  uint64_t length;   // LENGTH OF THE INPUT SEQUENCE
  }
RCLASS;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void      ShiftRBuf          (uint8_t *, uint32_t, uint8_t);
uint8_t   GetNBase           (uint8_t *, uint64_t);
RCLASS    *CreateRC          (uint32_t, double, double, uint32_t, uint32_t, 
                             double, uint8_t);
uint64_t  GetIdxRev          (uint8_t *, RCLASS *);
uint64_t  GetIdx             (uint8_t *, RCLASS *);
RENTRY    *GetHEnt           (RCLASS *, uint64_t);
int32_t   StartRM            (RCLASS *, uint32_t, uint64_t, uint8_t);
void      InsertKmerPos      (RCLASS *, uint64_t, uint32_t);
void      ComputeRMProbs     (RCLASS *, RMODEL *, uint8_t *);
void      UpdateRM           (RMODEL *, uint8_t *, uint8_t);
void      RenormWeights      (RCLASS *);
void      StopRM             (RCLASS *);
void      StartMultipleRMs   (RCLASS *, uint8_t *);
void      ComputeMixture     (RCLASS *, PMODEL *, uint8_t *);
void      UpdateWeights      (RCLASS *, uint8_t *, uint8_t);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
