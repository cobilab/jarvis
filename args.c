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

ModelPar ArgsUniqModel(char *str, uint8_t type){
  uint32_t  ctx, den, edits, eDen;
  double    gamma, eGamma;
  ModelPar  Mp;

  if(sscanf(str, "%u:%u:%lf/%u:%u:%lf", &ctx, &den, &gamma, &edits, &eDen, 
  &eGamma ) == 6){
    if(ctx > MAX_CTX || ctx < MIN_CTX || den > MAX_DEN || den < MIN_DEN || 
    gamma >= 1.0 || gamma < 0.0 || eGamma >= 1.0 || eGamma < 0.0 ||edits > 256 
    || eDen > 50000){
      fprintf(stderr, "Error: invalid model arguments range!\n");
      ModelsExplanation();
      fprintf(stderr, "\nPlease reset the models according to the above " 
      "description.\n");
      exit(1);
      }
    Mp.ctx    = ctx;
    Mp.den    = den;
    Mp.gamma  = ((int)(gamma * 65534)) / 65534.0;
    Mp.eGamma = ((int)(eGamma * 65534)) / 65534.0;
    Mp.edits  = edits;
    Mp.eDen   = eDen;
    Mp.type   = type;
    return Mp;
    }
  else{
    fprintf(stderr, "Error: unknown scheme for model arguments!\n");
    ModelsExplanation();
    fprintf(stderr, "\nPlease reset the models according to the above "
    "description.\n");
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

  fprintf(stderr, "Force mode ......................... %s\n", P->force == 0 ? 
  "no" : "yes");

  for(n = 0 ; n < P->nModels ; ++n)
    if(P->model[n].type == 1){
      fprintf(stderr, "Reference model %d:\n", n+1);
      fprintf(stderr, "  [+] Context order ................ %u\n", 
      P->model[n].ctx);
      fprintf(stderr, "  [+] Alpha denominator ............ %u\n", 
      P->model[n].den);
      fprintf(stderr, "  [+] Gamma ........................ %.2lf\n", 
      P->model[n].gamma);
      fprintf(stderr, "  [+] Allowable substitutions ...... %u\n",
      P->model[n].edits);
      if(P->model[n].edits != 0){
        fprintf(stderr, "  [+] Substitutions alpha den ...... %u\n",
        P->model[n].eDen);
        fprintf(stderr, "  [+] Substitutions gamma .......... %.2lf\n",
        P->model[n].eGamma);
        }
    }

  for(n = 0 ; n < P->nModels ; ++n)
    if(P->model[n].type == 0){
      fprintf(stderr, "Target model %d:\n", n+1);
      fprintf(stderr, "  [+] Context order ................ %u\n",
      P->model[n].ctx);
      fprintf(stderr, "  [+] Alpha denominator ............ %u\n",
      P->model[n].den);
      fprintf(stderr, "  [+] Gamma ........................ %.2lf\n", 
      P->model[n].gamma);
      fprintf(stderr, "  [+] Allowable substitutions ...... %u\n",
      P->model[n].edits);
      if(P->model[n].edits != 0){
        fprintf(stderr, "  [+] Substitutions alpha den ...... %u\n",
        P->model[n].eDen);
        fprintf(stderr, "  [+] Substitutions gamma .......... %.2lf\n",
        P->model[n].eGamma);
        }
      }

  if(P->ref != NULL)
    fprintf(stderr, "Reference filename ................. %s\n", P->ref);
  fprintf(stderr, "Target files (%u):\n", P->nTar);
  for(n = 0 ; n < P->nTar ; ++n)
    fprintf(stderr, "  [+] Filename %-2u .................. %s\n", n + 1, 
    P->tar[n]);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
