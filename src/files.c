#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "files.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FILE *Fopen(const char *path, const char *mode){
  FILE *F = fopen(path, mode);
  if(F == NULL){
    fprintf(stderr, "Error opening: %s (mode %s). Does the file exist?\n",
    path, mode);
    exit(1);
    }
  return F;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t NBytesInFile(FILE *F){
  uint64_t size = 0;
  fseeko(F, 0, SEEK_END);
  size = ftello(F);
  rewind(F);
  return size;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t FopenBytesInFile(const char *fn){
  uint64_t size = 0;
  FILE *F = Fopen(fn, "r");
  size = NBytesInFile(F);  
  fclose(F);
  return size;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FAccessWPerm(char *fn){
  if(access(fn, F_OK) != -1){
    fprintf(stderr, "Error: file %s already exists!\n", fn);
    if(access(fn, W_OK) != -1)
      fprintf(stderr, "Note: file %s has write permission.\nTip: to force "
      "writing rerun with \"-f\" option.\nWarning: on forcing, the old (%s) "
      "file will be deleted permanently.\n", fn, fn);
    exit(1);
    }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void TestReadFile(char *fn){
  FILE *F = NULL;
  F = Fopen(fn, "r");
  fclose(F);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
