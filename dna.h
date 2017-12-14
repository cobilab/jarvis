#ifndef DNA_H_INCLUDED
#define DNA_H_INCLUDED

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t  NDNASyminFile   (FILE *);
uint8_t   S2N             (uint8_t);
uint8_t   N2S             (uint8_t);
uint8_t   CompSym         (uint8_t);
uint8_t   CompNum         (uint8_t);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
