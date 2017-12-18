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

ModelPar ArgsUniqCModel(char *str, uint8_t type){
  uint32_t  ctx, den, eDen, ir, eIr, edits;
  double    gamma, eGamma;
  ModelPar  Mp;

  if(sscanf(str, "%u:%u:%u:%lf/%u:%u:%u:%lf", 
    &ctx, &den, &ir, &gamma, &edits, &eDen, &eIr, &eGamma) == 8){

    if(ctx    >  MAX_CTX || ctx    < MIN_CTX ||
       den    >  MAX_DEN || den    < MIN_DEN || 
       gamma  >= 1.0     || gamma  < 0.0     || 
       eGamma >= 1.0     || eGamma < 0.0     ||
       edits  >  256     || eDen   > 50000   ||
       ir     >  1       || ir     < 0       ||
       eIr    >  1       || eIr    < 0){
       FailModelScheme();
       exit(1);
       }

    Mp.ctx    = ctx;
    Mp.den    = den;
    Mp.gamma  = ((int)(gamma  * 65534)) / 65534.0;
    Mp.eGamma = ((int)(eGamma * 65534)) / 65534.0;
    Mp.edits  = edits;
    Mp.eDen   = eDen;
    Mp.type   = type;
    Mp.ir     = ir;
    Mp.eIr    = eIr;
    Mp.copy   = 0;

    return Mp;
    }
  else{
    FailModelScheme();
    exit(1);
    }

  return Mp;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ModelPar ArgsUniqRModel(char *str, uint8_t type){
  uint32_t  m, ctx, limit, ir;
  double    alpha, beta, gamma;
  ModelPar  Mp;

  if(sscanf(str, "%u:%u:%lf:%lf:%u:%lf:%u", 
  &m, &ctx, &alpha, &beta, &limit, &gamma, &ir) == 7){

    if(m      >  100000  || m      <  1       ||
       ctx    >  MAX_CTX || ctx    <  MIN_CTX ||
       alpha  >  1       || alpha  <= 0       || 
       beta   >= 1       || beta   <= 0       ||
       limit  >  21      || limit  <= 0       ||
       gamma  >= 1       || gamma  <= 0       ||
       ir     >  1       || ir     <  0){
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
    Mp.type   = 1;
    Mp.copy   = 1;

    return Mp;
    }
  else{
    FailModelScheme();
    exit(1);
    }

  return Mp;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t ReadFNames(Parameters *P, char *arg){
  uint32_t nFiles = 1, k = 0, argLen;
  
  argLen = strlen(arg);
  for( ; k != argLen ; ++k)
    if(arg[k] == ':')
      ++nFiles;
  P->tar = (char **) Malloc(nFiles * sizeof(char *));
  P->tar[0] = strtok(arg, ":");
  TestReadFile(P->tar[0]);
  for(k = 1 ; k != nFiles ; ++k){
    P->tar[k] = strtok(NULL, ":");
    TestReadFile(P->tar[k]);
    }

  return nFiles;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintArgs(Parameters *P){
  uint32_t n = 0;

  fprintf(stderr, "Verbose mode ....................... %s\n", !P->verbose ?
  "no" : "yes"); 
  fprintf(stderr, "Force mode ......................... %s\n", !P->force ? 
  "no" : "yes");

  for(n = 0 ; n < P->nModels ; ++n)
    if(P->model[n].copy == 0){
      fprintf(stderr, "Context model %d:\n", n+1);
      fprintf(stderr, "  [+] Context order (depth) ........ %u\n", 
      P->model[n].ctx);
      fprintf(stderr, "  [+] Alpha ........................ %.3lf\n", 
      1.0 / P->model[n].den);
      fprintf(stderr, "  [+] Gamma ........................ %.3lf\n", 
      P->model[n].gamma);
      fprintf(stderr, "  [+] Using inversions ............. %s\n",
      P->model[n].ir == 1 ? "yes" : "no"); 
      if(P->model[n].edits > 0){
        fprintf(stderr, "Substitutional tolerant context model:\n");
        fprintf(stderr, "  [+] Context order (depth) ........ %u\n",
        P->model[n].ctx);
        fprintf(stderr, "  [+] Allowable substitutions ...... %u\n",
        P->model[n].edits);
        fprintf(stderr, "  [+] Alpha ........................ %.3lf\n",
        1.0 / P->model[n].eDen);
        fprintf(stderr, "  [+] Gamma ........................ %.3lf\n",
        P->model[n].eGamma);
        fprintf(stderr, "  [+] Using inversions ............. %s\n",                    
        P->model[n].eIr == 1 ? "yes" : "no");
        }
      }

  for(n = 0 ; n < P->nModels ; ++n)
    if(P->model[n].copy == 1){
      fprintf(stderr, "Repeat model %d:\n", n+1);
      fprintf(stderr, "  [+] Maximum number of repeats .... %u\n",
      P->model[n].nr);
      fprintf(stderr, "  [+] Context order ................ %u\n",
      P->model[n].ctx);
      fprintf(stderr, "  [+] Alpha ........................ %.3lf\n",
      P->model[n].alpha);
      fprintf(stderr, "  [+] Beta ......................... %.3lf\n",
      P->model[n].beta);
      fprintf(stderr, "  [+] Gamma ........................ %.3lf\n", 
      P->model[n].gamma);
      fprintf(stderr, "  [+] Limit ........................ %u\n",
      P->model[n].limit);
      fprintf(stderr, "  [+] Using inversions ............. %s\n",
      P->model[n].ir == 1 ? "yes" : "no");
      }

  fprintf(stderr, "Target files (%u):\n", P->nTar);
  for(n = 0 ; n < P->nTar ; ++n)
    fprintf(stderr, "  [+] Filename %-2u .................. %s\n", n + 1, 
    P->tar[n]);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
