#ifndef FILES_H_INCLUDED
#define FILES_H_INCLUDED

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t     NBytesInFile       (FILE *);
uint64_t     FopenBytesInFile   (const char *);
FILE         *Fopen             (const char *, const char *);
void         FAccessWPerm       (char *);
void         TestReadFile       (char *);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
