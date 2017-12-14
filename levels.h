#ifndef LEVELS_H_INCLUDED
#define LEVELS_H_INCLUDED

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMPRESSION LEVELS FOR DNA 
//
#define LEVEL_1 " 1: -rm 20:1000:1:3/100 -g 0.95 "
#define LEVEL_2 " 2: -tm 4:1:0:0/0 -tm 11:10:0:0/0 -tm 13:20:1:0/0 -tm 19:20:1:2/10 -c 30 -g 0.85 "
#define LEVEL_3 " 3: -tm 1:1:0:0/0 -tm 4:1:0:0/0 -tm 6:1:0:0/0 -tm 8:1:0:0/0 -tm 12:10:0:0/0 -tm 13:20:1:1/10 -tm 18:100:1:6/10 -c 9 -g 0.90 "

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

char    *GetLevels  (uint8_t);
void    PrintLevels (void);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif

