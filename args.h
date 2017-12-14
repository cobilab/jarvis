#ifndef ARGS_H_INCLUDED
#define ARGS_H_INCLUDED

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t    ArgNumber        (uint32_t , char *[], uint32_t, char *, uint32_t,
                              uint32_t);
ModelPar    ArgsUniqModel    (char *, uint8_t);
ModelPar    ArgsModel        (uint32_t , char *[], uint32_t, char *);
double      ArgDouble        (double, char *[], uint32_t, char *);
uint8_t     ArgState         (uint8_t  , char *[], uint32_t, char *);
char        *ArgString       (char    *, char *[], uint32_t, char *);
uint32_t    ReadFNames       (Parameters *, char *);
void        PrintArgs        (Parameters *);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
