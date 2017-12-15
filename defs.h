#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <unistd.h>

//#pragma pack(1)

#define DEBUG 1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// RUNNING OPTIMIZATIONS : MEMORY / SPEED

typedef struct{
  uint32_t ctx;
  uint32_t den;
  uint32_t ir;
  uint32_t edits;
  uint32_t eDen;
  uint8_t  type;
  double   beta;
  int32_t  limit;
  int32_t  copy;
  double   gamma;
  double   eGamma;
  }
ModelPar;

typedef struct{
  uint8_t  help;
  uint8_t  verbose;
  uint8_t  force;
  uint8_t  estim;
  uint8_t  level;
  uint8_t  mode;
  uint32_t col;
  ModelPar *model;
  char     *ref;
  char     **tar;
  uint8_t  nTar;
  uint64_t checksum;
  uint64_t size;
  uint32_t watermark;
  double   gamma;
  uint32_t nModels;
  }
Parameters;

uint32_t garbage;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define RELEASE                1
#define VERSION                1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define BUFFER_SIZE            262144      
#define PROGRESS_MIN           1000000
#define DEF_VERSION            0
#define DEFAULT_HELP           0
#define DEFAULT_VERBOSE        0
#define DEFAULT_FORCE          0
#define DEFAULT_LEVEL          5
#define MAX_LEVEL              15
#define MIN_LEVEL              1
#define MAX_CTX                20
#define MIN_CTX                1
#define MAX_DEN                1000000
#define MIN_DEN                1
#define BGUARD                 32
#define DEFAULT_MAX_COUNT      ((1 << (sizeof(ACC) * 8)) - 1)
#define MX_PMODEL              65535
#define ALPHABET_SIZE          4
#define CHECKSUMGF             1073741824
#define WATERMARK              16042014
#define DEFAULT_GAMMA          0.90
#define MAX_HISTORYSIZE        1000000
#define MAX_STR                2048
#define REFERENCE              1
#define TARGET                 0

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif

