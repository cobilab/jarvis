#include "msg.h"
#include <stdio.h>
#include <stdlib.h>

void ModelsExplanation(void){
  fprintf(stderr,
  "                                                                       \n"
  "  -rm <r>:<c>:<a>:<b>:<l>:<g>:<i> RM (-rm 50:16:1:0.8:5:0.2:1),\n"
  "  -rm <r>:<c>:<a>:<b>:<l>:<g>:<i> RM (-rm 10:11:1:0.8:5:0.2:0),\n"
  "  ...                                                                  \n"
  "                         RM - Repeat Model parameters:                 \n"
  "                                                                       \n"
  "                         use <r> for the number of repeats, <c> for the\n"
  "                         context size, <a> for the alpha (estimator    \n"
  "                         parameter), <b> to set beta, <l> to set limit,\n"
  "                         <g> for gamma (decayment forgetting factor)   \n"
  "                         [0;1) and <i> to set the use or not of inveted\n"
  "                         repeat models,                                \n"
  "                                                                       \n"
  "  -cm <c>:<d>:<i>:<g>/<m>:<e>:<i>:<g> CM (-cm 1:1:0:0.8/0:0:0:0),\n"
  "  -cm <c>:<d>:<i>:<g>/<m>:<e>:<i>:<g> CM (-cm 13:100:1:0.9/1:10:0:0.9),\n"
  "  ...                                                                  \n"
  "                         CM - Context Model parameters:                \n"
  "                                                                       \n"
  "                         target and reference templates use <c> for    \n"
  "                         context-order size, <d> for alpha (1/<d>), <g>\n"
  "                         for gamma (decayment forgetting factor) [0;1),\n"
  "                         <m> to the maximum sets the allowed substitutions,\n"
  "                         on the context without being discarded (for   \n"
  "                         deep contexts), under the alpha (1/<d>), using\n"
  "                         <g> for gamma (decayment forgetting factor)   \n"
  "                         [0;1),                       \n");
  } 


void PrintMenu(void){
  fprintf(stderr,
  "                                                                       \n"
  "Jarvis: A tool to compress efficiently DNA sequences.                  \n"  
  "                                                                       \n"
  "Usage: Jarvis [OPTION]... -r [FILE]  [FILE]:[...]                      \n"
  "                                                                       \n"
  "Non-mandatory arguments:                                               \n"
  "                                                                       \n"
  "  -h                     give this help,                               \n"
  "  -s                     show Jarvis compression levels,               \n"
  "  -v                     verbose mode (more information),              \n"
  "  -V                     display version number,                       \n"
  "  -f                     force overwrite of output,                    \n"
  "  -l <level>             level of compression [1;3] (lazy -tm setup),  \n"
  "  -t <threshold>         threshold frequency to discard from alphabet,\n");
  #ifdef ESTIMATE
  fprintf(stderr,
  "  -e                     it creates a file with the extension \".iae\" \n"
  "                         with the respective information content.      \n");
  #endif
  ModelsExplanation();
  fprintf(stderr,
  "                                                                       \n"
  "  -r <FILE>              reference file (\"-rm\" are loaded here),     \n"
  "                                                                       \n"
  "Mandatory arguments:                                                   \n"
  "                                                                       \n"
  "  <FILE>:<...>:<...>     file to compress (last argument). For more    \n"
  "                         files use splitting \":\" characters.         \n"
  "                                                                       \n"
  "Example:                                                               \n"
  "                                                                       \n"
  "  [Encode] ./Jarvis -v -tm 1:1:0.8/0:0:0 -tm 5:20:0.9/3:20:0.9 seq.txt \n"
  "  [Decode] ./JArvis -v -d seq.txt.jc                                   \n"
  "                                                                       \n"
  "Report issues to <{pratas,ap}@ua.pt>.                                  \n"
  "                                                                       \n");
  }


void PrintVersion(void){
  fprintf(stderr,
  "                                                                       \n"
  "                          ================                             \n"
  "                          | Jarvis v%u.%u |                        \n"
  "                          ================                             \n"
  "                                                                       \n"
  "      Efficient compression and decompression of DNA sequences.        \n"
  "                                                                       \n"
  "Copyright (C) 2014-2018 University of Aveiro. This is a Free software. \n"
  "You may redistribute copies of it under the terms of the GNU - General \n"
  "Public License v3 <http://www.gnu.org/licenses/gpl.html>. There is NOT \n"
  "ANY WARRANTY, to the extent permitted by law. Developed and Written by \n"
  "Diogo Pratas.\n\n", VERSION, RELEASE);
  }


void MsgNoModels(void){
  fprintf(stderr, "Error: at least you need to use a context or repeat model!\n");
  }

