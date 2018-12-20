#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "args.h"
#include "mem.h"
#include "msg.h"
#include "strings.h"
#include "files.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t ArgNumber(uint32_t d, char *a[], uint32_t n, char *s, uint32_t l,
uint32_t u){
  uint32_t x;
  for( ; --n ; ) if(!strcmp(s, a[n])){
    if((x = atol(a[n+1])) < l || x > u){
      fprintf(stderr, "[x] Invalid number! Interval: [%u;%u].\n", l, u);
      exit(EXIT_FAILURE);
      }
    return x;
    }
  return d;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

double ArgDouble(double def, char *arg[], uint32_t n, char *str){
  for( ; --n ; )
    if(!strcmp(str, arg[n]))
      return atof(arg[n+1]);
  return def;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t ArgState(uint8_t def, char *arg[], uint32_t n, char *str){     
  for( ; --n ; )
    if(!strcmp(str, arg[n]))
      return def == 0 ? 1 : 0;
  return def;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

char *ArgString(char *def, char *arg[], uint32_t n, char *str){
  for( ; --n ; )
    if(!strcmp(str, arg[n]))
      return arg[n+1];
  return def;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CModelPar ArgsUniqCModel(char *str, uint8_t type){
  uint32_t   ctx, den, eDen, ir, eIr, edits;
  double     gamma, eGamma;
  CModelPar  Mp;

  if(sscanf(str, "%u:%u:%u:%lf/%u:%u:%u:%lf", 
    &ctx, &den, &ir, &gamma, &edits, &eDen, &eIr, &eGamma) == 8){

    if(ctx    >  MAX_CTX || ctx    < MIN_CTX ||
       den    >  MAX_DEN || den    < MIN_DEN || 
       gamma  >= 1.0     || gamma  < 0.0     || 
       eGamma >= 1.0     || eGamma < 0.0     ||
       edits  >  256     || eDen   > 50000   ||
       ir     >  1       ||
       eIr    >  1){
       FailModelScheme();
       exit(1);
       }

    Mp.ctx    = ctx;
    Mp.den    = den;
    Mp.ir     = ir;
    Mp.gamma  = ((int)(gamma  * 65534)) / 65534.0;
    Mp.eGamma = ((int)(eGamma * 65534)) / 65534.0;
    Mp.edits  = edits;
    Mp.eDen   = eDen;
    Mp.eIr    = eIr;

    return Mp;
    }
  else{
    FailModelScheme();
    exit(1);
    }

  return Mp;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

RModelPar ArgsUniqRModel(char *str, uint8_t type){
  uint32_t   m, ctx, limit, ir;
  double     alpha, beta, gamma;
  RModelPar  Mp;

  if(sscanf(str, "%u:%u:%lf:%lf:%u:%lf:%u", 
  &m, &ctx, &alpha, &beta, &limit, &gamma, &ir) == 7){

    if(m      >  100000  || m      <  1       ||
       ctx    >  MAX_CTX || ctx    <  MIN_CTX ||
       alpha  >  1       || alpha  <= 0       || 
       beta   >= 1       || beta   <= 0       ||
       limit  >  21      || limit  <= 0       ||
       gamma  >= 1       || gamma  <= 0       ||
       ir     >  1){
       FailModelScheme();
       exit(1);
       }
       
    Mp.nr     = m;
    Mp.ctx    = ctx;
    Mp.alpha  = ((int)(alpha * 65534)) / 65534.0;
    Mp.beta   = ((int)(beta  * 65534)) / 65534.0;
    Mp.gamma  = ((int)(gamma * 65534)) / 65534.0;
    Mp.limit  = limit;
    Mp.ir     = ir;

    return Mp;
    }
  else{
    FailModelScheme();
    exit(1);
    }

  return Mp;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintArgs(PARAM *P){
  uint32_t n = 0;

  fprintf(stderr, "Verbose mode ....................... %s\n", !P->verbose ?
  "no" : "yes"); 
  fprintf(stderr, "Force mode ......................... %s\n", !P->force ? 
  "no" : "yes");
  fprintf(stderr, "Predictive class context order ..... %u\n", P->selection);  

  for(n = 0 ; n < P->nCModels ; ++n){
    fprintf(stderr, "Context model %d:\n", n+1);
    fprintf(stderr, "  [+] Context order (depth) ........ %u\n",  
    P->cmodel[n].ctx);
    fprintf(stderr, "  [+] Alpha ........................ %.3lf\n", 
    1.0 / P->cmodel[n].den);
    fprintf(stderr, "  [+] Gamma ........................ %.3lf\n", 
    P->cmodel[n].gamma);
    fprintf(stderr, "  [+] Using inversions ............. %s\n",
    P->cmodel[n].ir == 1 ? "yes" : "no"); 
    if(P->cmodel[n].edits > 0){
      fprintf(stderr, "Substitutional tolerant context model:\n");
      fprintf(stderr, "  [+] Context order (depth) ........ %u\n",
      P->cmodel[n].ctx);
      fprintf(stderr, "  [+] Allowable substitutions ...... %u\n",
      P->cmodel[n].edits);
      fprintf(stderr, "  [+] Alpha ........................ %.3lf\n",
      1.0 / P->cmodel[n].eDen);
      fprintf(stderr, "  [+] Gamma ........................ %.3lf\n",
      P->cmodel[n].eGamma);
      fprintf(stderr, "  [+] Using inversions ............. %s\n",                    
      P->cmodel[n].eIr == 1 ? "yes" : "no");
      }
    }

  for(n = 0 ; n < P->nRModels ; ++n){
    fprintf(stderr, "Repeat model %d:\n", n+1);
    fprintf(stderr, "  [+] Maximum number of repeats .... %u\n",
    P->rmodel[n].nr);
    fprintf(stderr, "  [+] Context order ................ %u\n",
    P->rmodel[n].ctx);
    fprintf(stderr, "  [+] Alpha ........................ %.3lf\n",
    P->rmodel[n].alpha);
    fprintf(stderr, "  [+] Beta ......................... %.3lf\n",
    P->rmodel[n].beta);
    fprintf(stderr, "  [+] Gamma ........................ %.3lf\n", 
    P->rmodel[n].gamma);
    fprintf(stderr, "  [+] Limit ........................ %u\n",
    P->rmodel[n].limit);
    fprintf(stderr, "  [+] Using inversions ............. %s\n",
    P->rmodel[n].ir == 1 ? "yes" : "no");
    }

  fprintf(stderr, "Target file ........................ %s\n", P->tar); 
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
