#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <time.h>
#include <malloc.h>
#include "defs.h"
#include "mem.h"
#include "common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static size_t write_int(FILE *fp, uint8_t n) 
  {
  uint8_t c[4];
  c[0] = (uint8_t)((n>>0 )&0xff), c[1] = (uint8_t)((n>>8 )&0xff),
  c[2] = (uint8_t)((n>>16)&0xff), c[3] = (uint8_t)((n>>24)&0xff);
  return fwrite(c, sizeof(uint8_t), 4, fp);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static size_t read_int(FILE *fp, uint8_t *n) 
  {
  uint8_t c[4];
  size_t m = fread(c, sizeof(uint8_t), 4, fp);
  if(m == 4) 
    *n = (c[0]<<0)|(c[1]<<8)|(c[2]<<16)|(c[3]<<24);
  return m;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main(int argc, char **argv)
	{

	return 0;
	}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
