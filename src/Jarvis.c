//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                    J A R V I S   2 0 1 4 - 2 0 1 9                       //
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
// ENCODE HEADER
//
void EncodeHeader(PARAM *P, RCLASS **RC, CMODEL **CM, FILE *F){
  uint32_t n;

  WriteNBits(P->size,                                  SIZE_BITS, F);
  WriteNBits(P->length,                              LENGTH_BITS, F);
  WriteNBits(P->selection,                        SELECTION_BITS, F);

  WriteNBits(P->nCModels,                          NCMODELS_BITS, F);
  for(n = 0 ; n < P->nCModels ; ++n){
    WriteNBits(CM[n]->ctx,                              CTX_BITS, F);
    WriteNBits(CM[n]->alphaDen,                   ALPHA_DEN_BITS, F);
    WriteNBits((int)(CM[n]->gamma * 65534),           GAMMA_BITS, F);
    WriteNBits(CM[n]->ir,                                IR_BITS, F);
    WriteNBits(CM[n]->edits,                          EDITS_BITS, F);
    if(CM[n]->edits != 0){
      WriteNBits((int)(CM[n]->eGamma * 65534),      E_GAMMA_BITS, F);
      WriteNBits(CM[n]->TM->den,                      E_DEN_BITS, F);
      WriteNBits(CM[n]->TM->ir,                          IR_BITS, F);
      }
    }
  WriteNBits(P->nCPModels,                         NCMODELS_BITS, F);

  WriteNBits(P->nRModels,                          NRMODELS_BITS, F);
  for(n = 0 ; n < P->nRModels ; ++n){
    WriteNBits(RC[n]->mRM,                      MAX_RMODELS_BITS, F);
    WriteNBits((uint16_t)(RC[n]->P->alpha * 65534),   ALPHA_BITS, F);
    WriteNBits((uint16_t)(RC[n]->P->beta  * 65534),    BETA_BITS, F);
    WriteNBits((uint16_t)(RC[n]->P->gamma * 65534),   GAMMA_BITS, F);
    WriteNBits(RC[n]->P->limit,                       LIMIT_BITS, F);
    WriteNBits(RC[n]->P->ctx,                           CTX_BITS, F);
    WriteNBits(RC[n]->P->rev,                            IR_BITS, F);
    }

  #ifdef DEBUG
  printf("size    = %"PRIu64"\n", P->size);
  printf("length  = %"PRIu64"\n", P->length);
  printf("select  = %u\n",        P->selection);
  printf("n CMs   = %u\n",        P->nCModels);
  printf("n CPMs  = %u\n",        P->nCPModels);
  for(n = 0 ; n < P->nCModels ; ++n){
    printf("  cmodel %u\n",        n + 1);
    printf("    ctx       = %u\n", CM[n]->ctx);
    printf("    alpha den = %u\n", CM[n]->alphaDen);
    printf("    gamma     = %g\n", CM[n]->gamma);
    printf("    ir        = %u\n", CM[n]->ir);
    printf("    edits     = %u\n", CM[n]->edits);
    if(CM[n]->edits != 0){
      printf("      eGamma = %g\n", CM[n]->eGamma);
      printf("      eDen   = %u\n", CM[n]->TM->den);
      printf("      eIr    = %u\n", CM[n]->TM->ir);
      }
    }
  printf("n class = %u\n",        P->nRModels);
  for(n = 0 ; n < P->nRModels ; ++n){
    printf("  class %u\n",        n);
    printf("    max rep = %u\n",  RC[n]->mRM);
    printf("    alpha   = %g\n",  RC[n]->P->alpha);
    printf("    beta    = %g\n",  RC[n]->P->beta);
    printf("    gamma   = %g\n",  RC[n]->P->gamma);
    printf("    limit   = %u\n",  RC[n]->P->limit);
    printf("    ctx     = %u\n",  RC[n]->P->ctx);
    printf("    ir      = %u\n",  RC[n]->P->rev);
    }
  #endif
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMPRESSION
//
void Compress(PARAM *P, char *fn){
  FILE      *IN  = Fopen(fn, "r"), *OUT = Fopen(Cat(fn, ".jc"), "w");
  uint64_t  i = 0, mSize = MAX_BUF, pos = 0, r = 0;
  uint32_t  m, n, c, best;
  uint8_t   t[NSYM], *buf   = (uint8_t *) Calloc(mSize,    sizeof(uint8_t)), 
            sym = 0, *cache = (uint8_t *) Calloc(SCACHE+1, sizeof(uint8_t)),
            *p, irSym;

  RCLASS    **RC;
  CMODEL    **CM;
  PMODEL    **PM;
  PMODEL    *MX_CM;
  PMODEL    *MX_RM;
  FPMODEL   *PT;
  CMWEIGHT  *WM;
  CBUF      *SB;

  CMODEL    *AM     = CreateCModel(P->selection, 1, 1, 0, 0, 2, 0, 0, 0, 0);
  CBUF      *AM_BUF = CreateCBuffer(BUFFER_SIZE, BGUARD);
  PMODEL    *AM_PM  = CreatePModel(2);

  srand(0);

  if(P->verbose)
    fprintf(stderr, "Analyzing data and creating models ...\n");
 

  if(P->nCModels < 1 || P->nRModels < 1)
    {
    fprintf(stderr, "Error: At least one context and one repeat model must be set!\n");
    exit(1);    
    }

  #ifdef ESTIMATE
  FILE *IAE = NULL;
  char *IAEName = NULL;
  if(P->estim == 1){
    IAEName = Cat(fn, ".info");
    IAE = Fopen(IAEName, "w");
    }
  #endif

  // EXTRA MODELS DERIVED FROM TOLERANT CONTEXT MODELS
  P->nCPModels = P->nCModels;
  for(n = 0 ; n < P->nCModels ; ++n)
    if(P->cmodel[n].edits != 0)
      P->nCPModels++;

  PM      = (PMODEL  **) Calloc(P->nCPModels, sizeof(PMODEL *));
  for(n = 0 ; n < P->nCPModels ; ++n)
    PM[n] = CreatePModel(NSYM);
  MX_RM   = CreatePModel(NSYM);
  MX_CM   = CreatePModel(NSYM);
  PT      = CreateFloatPModel(NSYM);
  WM      = CreateWeightModel(P->nCPModels);
  SB      = CreateCBuffer(BUFFER_SIZE, BGUARD);

  CM = (CMODEL **) Malloc(P->nCModels * sizeof(CMODEL *));
  for(n = 0, r = 0; n < P->nCModels ; ++n){
    CM[n] = CreateCModel(P->cmodel[n].ctx,   P->cmodel[n].den,  1,
                         P->cmodel[n].edits, P->cmodel[n].eDen, NSYM,
                         P->cmodel[n].gamma, P->cmodel[n].eGamma,
                         P->cmodel[n].ir,    P->cmodel[n].eIr);
    // GIVE SPECIFIC GAMMA TO EACH MODEL:
    WM->gamma[r++] = CM[n]->gamma;
    if(CM[n]->edits != 0)
      WM->gamma[r++] = CM[n]->eGamma;
    }

  RC = (RCLASS **) Malloc(P->nRModels * sizeof(RCLASS *));
  for(n = 0 ; n < P->nRModels ; ++n)
    RC[n] = CreateRC(P->rmodel[n].nr,    P->rmodel[n].alpha, P->rmodel[n].beta,  
                     P->rmodel[n].limit, P->rmodel[n].ctx,   P->rmodel[n].gamma,
                     P->rmodel[n].ir);

  P->length = NBytesInFile(IN);
  P->size   = P->length>>2;

  if(P->verbose){
    fprintf(stderr, "Done!\n");
    fprintf(stderr, "Compressing %"PRIu64" symbols ...\n", P->size);
    }

  startoutputtingbits();
  start_encode();
  EncodeHeader(P, RC, CM, OUT);

  while((m = fread(t, sizeof(uint8_t), NSYM, IN)) == NSYM){
    buf[i] = S2N(t[3])|(S2N(t[2])<<2)|(S2N(t[1])<<4)|(S2N(t[0])<<6); // PACK 4
    
    for(n = 0 ; n < m ; ++n){

      SB->buf[SB->idx] = sym = S2N(t[n]);

      memset((void *)PT->freqs, 0, NSYM * sizeof(double));
      p = &SB->buf[SB->idx-1];

      for(r = 0 ; r < P->nRModels ; ++r){             // FOR ALL REPEAT MODELS
        StopRM           (RC[r]);
        StartMultipleRMs (RC[r], cache+SCACHE-1);
        InsertKmerPos    (RC[r], RC[r]->P->idx, pos);        // pos = (i<<2)+n
        RenormWeights    (RC[r]);
        ComputeMixture   (RC[r], MX_RM, buf);
        }

      for(r = 0, c = 0 ; r < P->nCModels ; ++r, ++c){       // FOR ALL CMODELS
        CMODEL *FCM = CM[r];
        GetPModelIdx(p, FCM);
        ComputePModel(FCM, PM[c], FCM->pModelIdx, FCM->alphaDen);
        ComputeWeightedFreqs(WM->weight[c], PM[c], PT, NSYM);
        if(FCM->edits != 0){
          FCM->TM->seq->buf[FCM->TM->seq->idx] = sym;
          FCM->TM->idx = GetPModelIdxCorr(FCM->TM->seq->buf+
          FCM->TM->seq->idx-1, FCM, FCM->TM->idx);
          ComputePModel(FCM, PM[++c], FCM->TM->idx, FCM->TM->den);
          ComputeWeightedFreqs(WM->weight[c], PM[c], PT, FCM->nSym);
          }
        }
      ComputeMXProbs(PT, MX_CM, NSYM);
      
      ++pos;

      best = (PModelStat(MX_CM, sym) < PModelStat(MX_RM, sym)) ? 0 : 1;
      AM_BUF->buf[AM_BUF->idx] = best;
      GetPModelIdx(&AM_BUF->buf[AM_BUF->idx-1], AM);
      ComputePModel(AM, AM_PM, AM->pModelIdx, AM->alphaDen);
 
      if(AM_PM->freqs[0] > AM_PM->freqs[1]){
        AESym(sym, (int *) (MX_CM->freqs), (int) MX_CM->sum, OUT);
        #ifdef ESTIMATE
        if(P->estim != 0)
          fprintf(IAE, "%.3g\n", PModelNats(MX_CM, sym) / M_LN2);
        #endif
        }
      else{
        AESym(sym, (int *) (MX_RM->freqs), (int) MX_RM->sum, OUT);
        #ifdef ESTIMATE
        if(P->estim != 0)
          fprintf(IAE, "%.3g\n", PModelNats(MX_RM, sym) / M_LN2);
        #endif
        }

      UpdateCModelCounter(AM, best, AM->pModelIdx);
      UpdateCBuffer(AM_BUF);

      CalcDecayment(WM, PM, sym);
      for(r = 0 ; r < P->nCModels ; ++r){
        UpdateCModelCounter(CM[r], sym, CM[r]->pModelIdx);
        if(CM[r]->ir != 0){                // REVERSE COMPLEMENTS
          irSym = GetPModelIdxIR(SB->buf+SB->idx, CM[r]);
          UpdateCModelCounter(CM[r], irSym, CM[r]->pModelIdxIR);
          }
        }
      RenormalizeWeights(WM);

      for(r = 0, c = 0 ; r < P->nCModels ; ++r, ++c)
        if(CM[r]->edits != 0)
          UpdateTolerantModel(CM[r]->TM, PM[++c], sym);

      for(r = 0 ; r < P->nRModels ; ++r)
        UpdateWeights(RC[r], buf, sym);

      ShiftRBuf(cache, SCACHE, sym);  // STORE THE LAST SCACHE BASES & SHIFT 1
      UpdateCBuffer(SB);
      }

    if(++i == mSize)    // REALLOC BUFFER ON OVERFLOW 4 STORE THE COMPLETE SEQ
      buf = (uint8_t *) Realloc(buf, (mSize+=mSize) * sizeof(uint8_t));

    Progress(P->size, i); 
    }

  WriteNBits(m, 8, OUT);
  for(n = 0 ; n < m ; ++n)
    WriteNBits(S2N(t[n]), 8, OUT);        // ENCODE REMAINING SYMBOLS

  fprintf(stderr, "Done!                                               \n");
  fprintf(stderr, "Compression: %"PRIu64" -> %"PRIu64" ( %.6g )\n", P->length, 
  (uint64_t) _bytes_output, (double) _bytes_output * 8.0 / P->length);

  finish_encode(OUT);
  doneoutputtingbits(OUT);

  #ifdef ESTIMATE
  if(P->estim == 1){
    fclose(IAE);
    Free(IAEName);
    }
  #endif

  fclose(IN);
  fclose(OUT);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// DECOMPRESSION
//
void Decompress(char *fn){
  FILE     *IN  = Fopen(fn, "r"), *OUT = Fopen(Cat(fn, ".jd"), "w");
  uint64_t i = 0, mSize = MAX_BUF, pos = 0;
  uint32_t m, n, r, c, best;
  uint8_t  *buf   = (uint8_t *) Calloc(mSize,    sizeof(uint8_t)),
           *cache = (uint8_t *) Calloc(SCACHE+1, sizeof(uint8_t)), 
           sym = 0, *p, irSym;
  RCLASS   **RC = NULL; 
  CMODEL   **CM = NULL;
  PARAM    *P = (PARAM *) Calloc(1, sizeof(PARAM));
  PMODEL   **PM;
  PMODEL   *MX_CM;
  PMODEL   *MX_RM;
  FPMODEL  *PT;
  CMWEIGHT *WM;
  CBUF     *SB;

  CMODEL   *AM;
  CBUF     *AM_BUF = CreateCBuffer(BUFFER_SIZE, BGUARD);
  PMODEL   *AM_PM  = CreatePModel(2);

  srand(0);

  startinputtingbits();
  start_decode(IN);

  P->size      = ReadNBits(                      SIZE_BITS, IN);
  P->length    = ReadNBits(                    LENGTH_BITS, IN);
  P->selection = ReadNBits(                 SELECTION_BITS, IN);
  P->nCModels  = ReadNBits(                  NCMODELS_BITS, IN);
  CM = (CMODEL **) Malloc(P->nCModels * sizeof(CMODEL *));
  for(n = 0 ; n < P->nCModels ; ++n){
    uint32_t c = ReadNBits(                       CTX_BITS, IN);
    uint32_t a = ReadNBits(                 ALPHA_DEN_BITS, IN);
    double   g = ReadNBits(                     GAMMA_BITS, IN) / 65534.0;
    uint32_t i = ReadNBits(                        IR_BITS, IN);
    uint32_t e = ReadNBits(                     EDITS_BITS, IN);
    uint32_t d = 0, y = 0;
    double   b = 0;
    if(e != 0){
      b = ReadNBits(                          E_GAMMA_BITS, IN) / 65534.0;
      d = ReadNBits(                            E_DEN_BITS, IN);
      y = ReadNBits(                               IR_BITS, IN);
      }
    CM[n] = CreateCModel(c, a, 1, e, d, NSYM, g, b, i, y);
    }
  P->nCPModels = ReadNBits(                  NCMODELS_BITS, IN);

  P->nRModels  = ReadNBits(                  NRMODELS_BITS, IN);
  RC = (RCLASS **) Malloc(P->nRModels * sizeof(RCLASS *));
  for(n = 0 ; n < P->nRModels ; ++n){
    uint32_t  m = ReadNBits(              MAX_RMODELS_BITS, IN);
    double    a = ReadNBits(                    ALPHA_BITS, IN) / 65534.0;
    double    b = ReadNBits(                     BETA_BITS, IN) / 65534.0;
    double    g = ReadNBits(                    GAMMA_BITS, IN) / 65534.0;
    uint32_t  l = ReadNBits(                    LIMIT_BITS, IN);
    uint32_t  c = ReadNBits(                      CTX_BITS, IN);
    uint8_t   r = ReadNBits(                       IR_BITS, IN);
    RC[n] = CreateRC(m, a, b, l, c, g, r);
    }

  #ifdef DEBUG
  printf("size    = %"PRIu64"\n", P->size);
  printf("length  = %"PRIu64"\n", P->length);
  printf("Select  = %u\n",        P->selection);
  printf("n CMs   = %u\n",        P->nCModels);
  printf("n CPMs  = %u\n",        P->nCPModels);
  for(n = 0 ; n < P->nCModels ; ++n){
    printf("  cmodel %u\n",        n + 1);
    printf("    ctx       = %u\n", CM[n]->ctx);
    printf("    alpha den = %u\n", CM[n]->alphaDen);
    printf("    gamma     = %g\n", CM[n]->gamma);
    printf("    ir        = %u\n", CM[n]->ir);
    printf("    edits     = %u\n", CM[n]->edits);
    if(CM[n]->edits != 0){
      printf("      eGamma = %g\n", CM[n]->eGamma);
      printf("      eDen   = %u\n", CM[n]->TM->den);
      printf("      eIr    = %u\n", CM[n]->TM->ir);
      }
    }
  printf("n class = %u\n",        P->nRModels);
  for(n = 0 ; n < P->nRModels ; ++n){
    printf("  class %u\n",        n + 1);
    printf("    max rep = %u\n",  RC[n]->mRM);
    printf("    alpha   = %g\n",  RC[n]->P->alpha);
    printf("    beta    = %g\n",  RC[n]->P->beta);
    printf("    gamma   = %g\n",  RC[n]->P->gamma);
    printf("    limit   = %u\n",  RC[n]->P->limit);
    printf("    ctx     = %u\n",  RC[n]->P->ctx);
    printf("    ir      = %u\n",  RC[n]->P->rev);
    }
  #endif

  AM      = CreateCModel(P->selection, 1, 1, 0, 0, 2, 0, 0, 0, 0);
  PM      = (PMODEL  **) Calloc(P->nCPModels, sizeof(PMODEL *));
  for(n = 0 ; n < P->nCPModels ; ++n)
    PM[n] = CreatePModel(NSYM);
  MX_RM   = CreatePModel(NSYM);
  MX_CM   = CreatePModel(NSYM);
  PT      = CreateFloatPModel(NSYM);
  WM      = CreateWeightModel(P->nCPModels);
  SB      = CreateCBuffer(BUFFER_SIZE, BGUARD);


  // GIVE SPECIFIC GAMMA TO EACH MODEL:
  for(n = 0, r = 0 ; n < P->nCModels ; ++n)
    {
    WM->gamma[r++] = CM[n]->gamma;
    if(CM[n]->edits != 0)
      WM->gamma[r++] = CM[n]->eGamma;
    }

  while(i < P->size){                         // NOT absolute size (CHAR SIZE)
    for(n = 0 ; n < NSYM ; ++n){

      memset((void *)PT->freqs, 0, NSYM * sizeof(double));
      p = &SB->buf[SB->idx-1];

      for(r = 0 ; r < P->nRModels ; ++r){
        StopRM           (RC[r]);
        StartMultipleRMs (RC[r], cache+SCACHE-1);
        InsertKmerPos    (RC[r], RC[r]->P->idx, pos);        // pos = (i<<2)+n
        RenormWeights    (RC[r]);
        ComputeMixture   (RC[r], MX_RM, buf);
        }

      for(r = 0, c = 0 ; r < P->nCModels ; ++r, ++c){       // FOR ALL CMODELS
        CMODEL *FCM = CM[r];
        GetPModelIdx(p, FCM);
        ComputePModel(FCM, PM[c], FCM->pModelIdx, FCM->alphaDen);
        ComputeWeightedFreqs(WM->weight[c], PM[c], PT, NSYM);
        if(FCM->edits != 0){
          FCM->TM->idx = GetPModelIdxCorr(FCM->TM->seq->buf+
          FCM->TM->seq->idx-1, FCM, FCM->TM->idx);
          ComputePModel(FCM, PM[++c], FCM->TM->idx, FCM->TM->den);
          ComputeWeightedFreqs(WM->weight[c], PM[c], PT, FCM->nSym);
          }
        }
      ComputeMXProbs(PT, MX_CM, NSYM);

      ++pos;

      GetPModelIdx(&AM_BUF->buf[AM_BUF->idx-1], AM);
      ComputePModel(AM, AM_PM, AM->pModelIdx, AM->alphaDen);

      sym = (AM_PM->freqs[0] > AM_PM->freqs[1]) ? 
      ArithDecodeSymbol(NSYM, (int *) MX_CM->freqs, (int) MX_CM->sum, IN):
      ArithDecodeSymbol(NSYM, (int *) MX_RM->freqs, (int) MX_RM->sum, IN);

      best = (PModelStat(MX_CM, sym) < PModelStat(MX_RM, sym)) ? 0 : 1;
      AM_BUF->buf[AM_BUF->idx] = best;

      UpdateCModelCounter(AM, best, AM->pModelIdx);
      UpdateCBuffer(AM_BUF);
      SB->buf[SB->idx] = sym;

      if(n == 0) buf[i] = sym<<6 ; else buf[i] |= (sym<<((3-n)<<1));
      fputc(N2S(sym), OUT);

      for(r = 0 ; r < P->nCModels ; ++r)
        if(CM[r]->edits != 0)
          CM[r]->TM->seq->buf[CM[r]->TM->seq->idx] = sym;

      CalcDecayment(WM, PM, sym);
      for(r = 0 ; r < P->nCModels ; ++r){
        UpdateCModelCounter(CM[r], sym, CM[r]->pModelIdx);
        if(CM[r]->ir != 0){                // REVERSE COMPLEMENTS
          irSym = GetPModelIdxIR(SB->buf+SB->idx, CM[r]);
          UpdateCModelCounter(CM[r], irSym, CM[r]->pModelIdxIR);
          }
        }
      RenormalizeWeights(WM);

      for(r = 0, c = 0 ; r < P->nCModels ; ++r, ++c)
        if(CM[r]->edits != 0)
          UpdateTolerantModel(CM[r]->TM, PM[++c], sym);

      for(r = 0 ; r < P->nRModels ; ++r)
        UpdateWeights(RC[r], buf, sym);

      ShiftRBuf(cache, SCACHE, sym);  // STORE THE LAST SCACHE BASES & SHIFT 1
      UpdateCBuffer(SB);
      }

    if(++i == mSize) // REALLOC BUFFER ON OVERFLOW 4 STORE THE COMPLETE SEQ
      buf = (uint8_t *) Realloc(buf, (mSize+=mSize) * sizeof(uint8_t));

    Progress(P->size, i);
    }

  m = ReadNBits(8, IN);
  for(n = 0 ; n < m ; ++n)
    fputc(N2S(ReadNBits(8, IN)), OUT);    // DECODE REMAINING SYMBOLS

  finish_decode();
  doneinputtingbits();

  RemoveCBuffer(AM_BUF);

  fclose(IN);
  fclose(OUT);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// MAIN 
//
int main(int argc, char **argv){
  char       **p = *&argv, **xargv, *xpl = NULL;
  int32_t    n, k, xargc = 0;
  PARAM      *P;
  clock_t    stop = 0, start = clock();
  
  P = (PARAM *) Calloc(1, sizeof(PARAM));

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

  P->verbose   = ArgState  (DEFAULT_VERBOSE,    p, argc, "-v" );
  P->force     = ArgState  (DEFAULT_FORCE,      p, argc, "-f" );
  P->estim     = ArgState  (0,                  p, argc, "-e" );
  P->selection = ArgNumber (0,                  p, argc, "-z", 1, 20);
  P->level     = ArgNumber (0,   p, argc, "-l", MIN_LEVEL, MAX_LEVEL);

  for(n = 1 ; n < argc ; ++n){
    if(strcmp(argv[n], "-cm") == 0){
      P->nCModels++;
      P->nModels++;
      }
    if(strcmp(argv[n], "-rm") == 0){
      P->nRModels++;
      P->nModels++;
      }
    }

  if(P->nModels == 0 && P->level == 0)
    P->level = DEFAULT_LEVEL;

  if(P->level != 0){
    xpl = GetLevels(P->level);
    xargc = StrToArgv(xpl, &xargv);
    for(n = 1 ; n < xargc ; ++n){
      if(strcmp(xargv[n], "-cm") == 0){
        P->nCModels++;
        P->nModels++;
        }
      if(strcmp(xargv[n], "-rm") == 0){
        P->nRModels++;
        P->nModels++;
        }
      }
    }

  if(P->nModels == 0 && !P->mode){
    MsgNoModels();
    return 1;
    }

  P->rmodel = (RModelPar *) Calloc(P->nRModels, sizeof(RModelPar));
  k = 0;
  for(n = 1 ; n < argc ; ++n)
    if(strcmp(argv[n], "-rm") == 0)
      P->rmodel[k++] = ArgsUniqRModel(argv[n+1], 0);
  if(P->level != 0){
    for(n = 1 ; n < xargc ; ++n)
      if(strcmp(xargv[n], "-rm") == 0)
        P->rmodel[k++] = ArgsUniqRModel(xargv[n+1], 0);
    }

  P->cmodel = (CModelPar *) Calloc(P->nCModels, sizeof(CModelPar));
  k = 0;
  for(n = 1 ; n < argc ; ++n)
    if(strcmp(argv[n], "-cm") == 0)
      P->cmodel[k++] = ArgsUniqCModel(argv[n+1], 0);
  if(P->level != 0){
    for(n = 1 ; n < xargc ; ++n)
      if(strcmp(xargv[n], "-cm") == 0)
        P->cmodel[k++] = ArgsUniqCModel(xargv[n+1], 0);
    }

  if(P->selection == 0){
    P->selection = DEFAULT_SELECTION;
    for(n = 1 ; n < xargc ; ++n)
      if(strcmp(xargv[n], "-z") == 0)
        P->selection = atoi(xargv[n+1]);
    }

  P->mode = ArgState (DEF_MODE,  p, argc, "-d"); // COMPRESS OR DECOMPRESS

  P->tar  = argv[argc-1];
 
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

  fprintf(stderr, "Done!                        \n");  // SPACES ARE VALID!
  return 0;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
