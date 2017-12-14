#include "msg.h"
#include <stdio.h>
#include <stdlib.h>

void ModelsExplanation(void){
  fprintf(stderr,
  "                                                                       \n"
  "  -rm <c>:<d>:<g>/<m>:<e>:<a>  reference model (-rm 1:10:0.9/0:0:0),   \n"
  "  -rm <c>:<d>:<g>/<m>:<e>:<a>  reference model (-rm 5:90:0.9/1:50:0.8),\n"
  "  ...                                                                  \n"
  "  -tm <c>:<d>:<g>/<m>:<e>:<a>  target model (-tm 1:1:0.8/0:0:0),       \n"
  "  -tm <c>:<d>:<g>/<m>:<e>:<a>  target model (-tm 7:100:0.9/2:10:0.85), \n"
  "  ...                                                                  \n"
  "                         target and reference templates use <c> for    \n"
  "                         context-order size, <d> for alpha (1/<d>), <g>\n"
  "                         for gamma (decayment forgetting factor) [0;1),\n"
  "                         <m> to the maximum sets the allowed mutations,\n"
  "                         on the context without being discarded (for   \n"
  "                         deep contexts), under the estimator <e>, using\n"
  "                         <a> for gamma (decayment forgetting factor)   \n"
  "                         [0;1) (tolerant model),                       \n");
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

