//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                    J A R V I S   2 0 1 4 - 2 0 1 8                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <time.h>
#include <malloc.h>
#include <unistd.h>
#include "defs.h"
#include "common.h"
#include "levels.h"
#include "dna.h"
#include "args.h"
#include "repeats.h"
#include "files.h"
#include "strings.h"
#include "mem.h"
#include "msg.h"
#include "bitio.h"
#include "arith.h"
#include "arith_aux.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// DECODE HEADER AN CREATE REPEAT CLASS
//
RCLASS *DecodeHeader(FILE *F){
  uint64_t s, w;
  uint32_t m, l, c;
  double   a, b, g;
  uint8_t  r;
  RCLASS   *C;
  s = ReadNBits(64, F);
  w = ReadNBits(64, F);
  m = ReadNBits(32, F);
  a = ReadNBits(16, F) / 65535.0;
  b = ReadNBits(16, F) / 65535.0;
  g = ReadNBits(16, F) / 65535.0;
  l = ReadNBits(16, F);
  c = ReadNBits(16, F);
  r = ReadNBits( 1, F);
  C = CreateRC(m, a, b, l, c, g, r);
  C->size   = s;
  C->length = w;

  #ifdef DEBUG
  printf("size    = %"PRIu64"\n", C->size);
  printf("length  = %"PRIu64"\n", C->length);
  printf("max rep = %u\n",        C->mRM);
  printf("alpha   = %g\n",        C->P->alpha);
  printf("beta    = %g\n",        C->P->beta);
  printf("gamma   = %g\n",        C->P->gamma);
  printf("limit   = %u\n",        C->P->limit);
  printf("ctx     = %u\n",        C->P->ctx);
  printf("ir      = %u\n",        C->P->rev);
  #endif

  return C;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ENCODE HEADER
//
void EncodeHeader(RCLASS *C, FILE *F){
  WriteNBits(C->size,                       64, F);
  WriteNBits(C->length,                     64, F);
  WriteNBits(C->mRM,                        32, F);
  WriteNBits((uint16_t)(C->P->alpha*65535), 16, F);
  WriteNBits((uint16_t)(C->P->beta *65535), 16, F);
  WriteNBits((uint16_t)(C->P->gamma*65535), 16, F);
  WriteNBits(C->P->limit,                   16, F);
  WriteNBits(C->P->ctx,                     16, F);
  WriteNBits(C->P->rev,                      1, F);
  
  #ifdef DEBUG
  printf("size    = %"PRIu64"\n", C->size);
  printf("length  = %"PRIu64"\n", C->length);
  printf("max rep = %u\n",        C->mRM);
  printf("alpha   = %g\n",        C->P->alpha);
  printf("beta    = %g\n",        C->P->beta);
  printf("gamma   = %g\n",        C->P->gamma);
  printf("limit   = %u\n",        C->P->limit);
  printf("ctx     = %u\n",        C->P->ctx);
  printf("ir      = %u\n",        C->P->rev);
  #endif
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMPRESSION
//
void Compress(RCLASS *C, char *fn){
  FILE     *IN  = Fopen(fn, "r"), *OUT = Fopen(Cat(fn, ".jc"), "w");
  uint64_t i = 0, mSize = MAX_BUF, pos = 0;
  uint32_t m, n; 
  uint8_t  t[NSYM], *buf = (uint8_t *) Calloc(mSize, sizeof(uint8_t)), 
           *cache = (uint8_t *) Calloc(SCACHE+1, sizeof(uint8_t)), sym = 0;
  PMODEL   *MX = CreatePM(NSYM);
  srand(0);

  C->length = NBytesInFile(IN);
  C->size   = C->length>>2;

  startoutputtingbits();
  start_encode();
  EncodeHeader(C, OUT);

  while((m = fread(t, sizeof(uint8_t), NSYM, IN)) == NSYM){
    buf[i] = S2N(t[3])|(S2N(t[2])<<2)|(S2N(t[1])<<4)|(S2N(t[0])<<6); // PACK 4
    
    for(n = 0 ; n < m ; ++n){
      sym = S2N(t[n]);
      StopRM(C);
      StartMultipleRMs(C, cache+SCACHE-1);
      InsertKmerPos(C, C->P->idx, pos++);                    // pos = (i<<2)+n
      RenormWeights(C);
      ComputeMixture(C, MX, buf);
      AESym(sym, (int *)(MX->freqs), (int) MX->sum, OUT);
      UpdateWeights(C, buf, sym);
      ShiftRBuf(cache, SCACHE, sym);  // STORE THE LAST SCACHE BASES & SHIFT 1
      }

    if(++i == mSize)    // REALLOC BUFFER ON OVERFLOW 4 STORE THE COMPLETE SEQ
      buf = (uint8_t *) Realloc(buf, (mSize+=mSize) * sizeof(uint8_t));

    Progress(C->size, i); 
    }

  WriteNBits(m, 8, OUT);
  for(n = 0 ; n < m ; ++n)
    WriteNBits(S2N(t[n]), 8, OUT);        // ENCODE REMAINING SYMBOLS

  fprintf(stderr, "Compression: %"PRIu64" -> %"PRIu64" ( %.6g )\n", C->length, 
  (uint64_t) _bytes_output, (double) _bytes_output * 8.0 / C->length);

  finish_encode(OUT);
  doneoutputtingbits(OUT);
  fclose(IN);
  fclose(OUT);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// DECOMPRESSION
//
void Decompress(char *fn){
  FILE     *IN  = Fopen(fn, "r"), *OUT = Fopen(Cat(fn, ".jd"), "w");
  uint64_t i = 0, mSize = MAX_BUF, pos = 0;
  uint32_t m, n;
  uint8_t  *buf = (uint8_t *) Calloc(mSize, sizeof(uint8_t)),
           *cache = (uint8_t *) Calloc(SCACHE+1, sizeof(uint8_t)), sym = 0;
  PMODEL   *MX = CreatePM(NSYM);
  srand(0);

  startinputtingbits();
  start_decode(IN);
  RCLASS *C = DecodeHeader(IN);

  while(i < C->size){                         // NOT absolute size (CHAR SIZE)
    for(n = 0 ; n < NSYM ; ++n){
      StopRM(C);
      StartMultipleRMs(C, cache+SCACHE-1);
      InsertKmerPos(C, C->P->idx, pos++);                    // pos = (i<<2)+n
      RenormWeights(C);
      ComputeMixture(C, MX, buf);
      sym = ArithDecodeSymbol(NSYM, (int *) MX->freqs, (int) MX->sum, IN);
      if(n == 0) buf[i] = sym<<6 ; else buf[i] |= (sym<<((3-n)<<1));
      fputc(N2S(sym), OUT);
      UpdateWeights(C, buf, sym);
      ShiftRBuf(cache, SCACHE, sym);  // STORE THE LAST SCACHE BASES & SHIFT 1
      }

    if(++i == mSize) // REALLOC BUFFER ON OVERFLOW 4 STORE THE COMPLETE SEQ
      buf = (uint8_t *) Realloc(buf, (mSize+=mSize) * sizeof(uint8_t));

    Progress(C->size, i);
    }

  m = ReadNBits(8, IN);
  for(n = 0 ; n < m ; ++n)
    fputc(N2S(ReadNBits(8, IN)), OUT);    // DECODE REMAINING SYMBOLS

  finish_decode();
  doneinputtingbits();
  fclose(IN);
  fclose(OUT);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// MAIN 
//
int main(int argc, char **argv){
  char       **p = *&argv, **xargv, *xpl = NULL, i, d;
  uint32_t   c, l, m;
  double     a, g, b;
  RCLASS     *C;
  Parameters *P;
  
  P = (Parameters *) Malloc(1 * sizeof(Parameters));

  if((P->help = ArgState(DEFAULT_HELP, p, argc, "-h")) == 1 || argc < 2){
    PrintMenu();
/*
    fprintf(stderr, "Usage: Jarvis [OPTIONS]... [FILE]              \n"); 
    fprintf(stderr, "                                               \n"); 
    fprintf(stderr, "  -m <REPEATS>  maximum number of repeats      \n"); 
    fprintf(stderr, "  -c <CONTEXT>  context order (k-mer)          \n"); 
    fprintf(stderr, "  -a <ALPHA>    alpha probabilities estimator  \n"); 
    fprintf(stderr, "  -b <BETA>     acceptance threshold           \n"); 
    fprintf(stderr, "  -l <LIMIT>    limit for acceptance threshold \n"); 
    fprintf(stderr, "  -g <GAMMA>    mixture decay factor           \n"); 
    fprintf(stderr, "  -i            use inverted repeats           \n"); 
    fprintf(stderr, "                                               \n"); 
    fprintf(stderr, "  -d            decompression mode             \n"); 
    fprintf(stderr, "                                               \n"); 
    fprintf(stderr, "  <FILE>        target file                    \n");
    fprintf(stderr, "                                               \n"); 
*/
    return EXIT_SUCCESS;
    }

  if(ArgState(DEF_VERSION, p, argc, "-V")){
    PrintVersion();
    return EXIT_SUCCESS;
    }

  if(ArgState(0, p, argc, "-s")){
    PrintLevels();
    return EXIT_SUCCESS;
    }

  P->verbose = ArgState  (DEFAULT_VERBOSE, p, argc, "-v" );
  P->force   = ArgState  (DEFAULT_FORCE,   p, argc, "-f" );
  P->estim   = ArgState  (0,               p, argc, "-e" );
  P->level   = ArgNumber (0,   p, argc, "-l", MIN_LEVEL, MAX_LEVEL);

  m = ArgNumber(DEF_MRM,   p, argc, "-m", 1, 20000);  // SEE THE HEAD OF THE FILE FOR THE 
  c = ArgNumber(DEF_CTX,   p, argc, "-c", 1,    31);  // DEFAULT& NAIVE ESTIMATED VALUES.
  a = ArgDouble(DEF_ALPHA, p, argc, "-a");            // THE ESTIMATION OF THE PARAMETERS
  g = ArgDouble(DEF_GAMMA, p, argc, "-g");            // IMPROVE SUBSTANTIALLY THE REPEAT
  b = ArgDouble(DEF_BETA,  p, argc, "-b");            // BASED COMPRESSION. AS SUCH, SOME
  l = ArgNumber(DEF_LIMIT, p, argc, "-l", 1,    50);  // TIME IN FUTURE WORKS MAY BE LOST
  i = ArgState (DEF_REV,   p, argc, "-i");            // FOR ESTIMATION OR IN INTELLIGENT
  d = ArgState (DEF_MODE,  p, argc, "-d");            // PREDICTION MODELLING DESIGN.
 
  if(!d){
    fprintf(stderr, "Compressing ...\n"); 
    C = CreateRC(m, a, b, l, c, g, i);
    Compress(C, argv[argc-1]);
    }
  else{
    fprintf(stderr, "Decompressing ...\n"); 
    Decompress(argv[argc-1]);
    }
 
  fprintf(stderr, "Done!\n");
  return 0;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
