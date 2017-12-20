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
#include "cm.h"
#include "pmodels.h"
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
void Compress(Parameters *P, char *fn){
  FILE     *IN  = Fopen(fn, "r"), *OUT = Fopen(Cat(fn, ".jc"), "w");
  uint64_t i = 0, mSize = MAX_BUF, pos = 0;
  uint32_t m, n; 
  uint8_t  t[NSYM], *buf = (uint8_t *) Calloc(mSize, sizeof(uint8_t)), 
           *cache = (uint8_t *) Calloc(SCACHE+1, sizeof(uint8_t)), sym = 0;
  RCLASS   **RC;
  CModel   **CM;
  PMODEL   **PM;
  PMODEL   *MX;
  FPMODEL  *PT;
  CMWEIGHT *WM;

  srand(0);

  if(P->verbose)
    fprintf(stderr, "Analyzing data and creating models ...\n");

  #ifdef ESTIMATE
  FILE *IAE = NULL;
  char *IAEName = NULL;
  if(P->estim == 1){
    IAEName = concatenate(P->tar[id], ".iae");
    IAE = Fopen(IAEName, "w");
    }
  #endif

  // EXTRA MODELS DERIVED FROM TOLERANT CONTEXT MODELS
  int32_t totModels = P->nModels;
  for(n = 0 ; n < P->nModels ; ++n)
    if(P->model[n].edits != 0)
      totModels++;

  PM      = (PMODEL  **) Calloc(totModels, sizeof(PMODEL *));
  for(n = 0 ; n < totModels ; ++n)
    PM[n] = CreatePModel(4);
  MX      = CreatePModel(4);
  PT      = CreateFloatPModel(4);
  WM      = CreateWeightModel(totModels);

  RC = (RCLASS **) Malloc(P->nModels * sizeof(RCLASS *));
  for(n = 0 ; n < P->nModels ; ++n)
    if(P->model[n].repeat == 1)
      RC[n] = CreateRC(P->model[n].nr,    P->model[n].alpha, P->model[n].beta,  
                       P->model[n].limit, P->model[n].ctx,   P->model[n].gamma,
                       P->model[n].ir);


  CM = (CModel **) Malloc(P->nModels * sizeof(CModel *));
  for(n = 0 ; n < P->nModels ; ++n)
    if(P->model[n].repeat == 0)
      CM[n] = CreateCModel(P->model[n].ctx,   P->model[n].den,  1, 
                           P->model[n].edits, P->model[n].eDen, 4, 
                           P->model[n].gamma, P->model[n].eGamma);

  C->length = NBytesInFile(IN);
  C->size   = C->length>>2;

  if(P->verbose){
    fprintf(stderr, "Done!\n");
    fprintf(stderr, "Compressing %"PRIu64" symbols ...\n", C->size);
    }


  startoutputtingbits();
  start_encode();
  EncodeHeader(RC, OUT);

  while((m = fread(t, sizeof(uint8_t), NSYM, IN)) == NSYM){
    buf[i] = S2N(t[3])|(S2N(t[2])<<2)|(S2N(t[1])<<4)|(S2N(t[0])<<6); // PACK 4
    
    for(n = 0 ; n < m ; ++n){
      sym = S2N(t[n]);
      for(r = 0 ; r < nr ; ++r){
        StopRM(RC[n]);
        StartMultipleRMs(RC[n], cache+SCACHE-1);
        InsertKmerPos(C, C->P->idx, pos++);                    // pos = (i<<2)+n
        RenormWeights(C);
        ComputeMixture(C, MX, buf);
        }

      AESym(sym, (int *)(MX->freqs), (int) MX->sum, OUT);

      for(r = 0 ; r < nr ; ++r)
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
  PMODEL   *MX = CreatePModel(NSYM);
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
  char       **p = *&argv, **xargv, *xpl = NULL;
  int32_t    n, k, xargc = 0;
  Parameters *P;
  clock_t    stop = 0, start = clock();
  
  P = (Parameters *) Malloc(1 * sizeof(Parameters));

  if((P->help = ArgState(DEFAULT_HELP, p, argc, "-h")) == 1 || argc < 2){
    PrintMenu();
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

  P->nModels = 0;
  for(n = 1 ; n < argc ; ++n)
    if(strcmp(argv[n], "-cm") == 0 || strcmp(argv[n], "-rm") == 0)
      P->nModels += 1;

  if(P->nModels == 0 && P->level == 0)
    P->level = DEFAULT_LEVEL;

  if(P->level != 0){
    xpl = GetLevels(P->level);
    xargc = StrToArgv(xpl, &xargv);
    for(n = 1 ; n < xargc ; ++n)
      if(strcmp(xargv[n], "-cm") == 0 || strcmp(xargv[n], "-rm") == 0)
        P->nModels += 1;
    }

  if(P->nModels == 0 && !P->mode){
    MsgNoModels();
    return 1;
    }

  P->model = (ModelPar *) Calloc(P->nModels, sizeof(ModelPar));

  k = 0;
  for(n = 1 ; n < argc ; ++n)
    if(strcmp(argv[n], "-rm") == 0)
      P->model[k++] = ArgsUniqRModel(argv[n+1], 0);
  if(P->level != 0){
    for(n = 1 ; n < xargc ; ++n)
      if(strcmp(xargv[n], "-rm") == 0)
        P->model[k++] = ArgsUniqRModel(xargv[n+1], 0);
    }
  for(n = 1 ; n < argc ; ++n)
    if(strcmp(argv[n], "-cm") == 0)
      P->model[k++] = ArgsUniqCModel(argv[n+1], 0);
  if(P->level != 0){
    for(n = 1 ; n < xargc ; ++n)
      if(strcmp(xargv[n], "-cm") == 0)
        P->model[k++] = ArgsUniqCModel(xargv[n+1], 0);
    }

  P->nTar = ReadFNames(P, argv[argc-1]);
  P->mode = ArgState (DEF_MODE,  p, argc, "-d"); // COMPRESS OR DECOMPRESS
 
  if(!P->mode){
    if(P->verbose) PrintArgs(P);
    fprintf(stderr, "Compressing ...\n"); 
    Compress(P, argv[argc-1]);
    }
  else{
    fprintf(stderr, "Decompressing ...\n"); 
    Decompress(argv[argc-1]);
    }

  stop = clock();
  if(P->verbose)
    fprintf(stderr, "Spent %g seconds.\n", ((double)(stop-start)) / 
    CLOCKS_PER_SEC); 

  fprintf(stderr, "Done!\n");
  return 0;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
