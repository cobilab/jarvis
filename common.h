#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include "defs.h"

typedef struct{
  uint64_t size;
  uint64_t bytes;
  uint64_t header;
  double   se;
  }
INF;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void        ShiftBuffer      (uint8_t *, int, uint8_t);
double      Power            (double, double);
double      Log              (double, double);
uint64_t    CalcMult         (uint32_t);
uint8_t     CmpCheckSum      (uint32_t, uint32_t);
void        Progress         (uint64_t , uint64_t);
int32_t     StrToArgv        (char *, char ***);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
