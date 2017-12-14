#ifndef STRINGS_H_INCLUDED
#define STRINGS_H_INCLUDED

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void        ShiftBuffer      (uint8_t *, int, uint8_t);
char        *ReplaceSubStr   (char *, char *, char *);
uint8_t     *ReverseStr      (uint8_t *, uint32_t);
char        *CloneString     (char *);
char        *Cat             (char *, char *);
int32_t     StrToArgv        (char *, char ***);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
