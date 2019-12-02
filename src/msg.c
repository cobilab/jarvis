#include "msg.h"
#include <stdio.h>
#include <stdlib.h>

void ModelsExplanation(void){
  fprintf(stderr,
  "                                                                   \n"
  "      -cm [NB_C]:[NB_D]:[NB_I]:[NB_G]/[NB_S]:[NB_E]:[NB_I]:[NB_A]  \n"
  "      Template of a context model.                                 \n"
  "      Parameters:                                                  \n"
  "      [NB_C]: (integer [1;20]) order size of the regular context   \n"
  "              model. Higher values use more RAM but, usually, are  \n"
  "              related to a better compression score.               \n"
  "      [NB_D]: (integer [1;5000]) denominator to build alpha, which \n"
  "              is a parameter estimator. Alpha is given by 1/[NB_D].\n"
  "              Higher values are usually used with higher [NB_C],   \n"
  "              and related to confiant bets. When [NB_D] is one,    \n"
  "              the probabilities assume a Laplacian distribution.   \n"
  "      [NB_I]: (integer {0,1}) number to define if a sub-program    \n"
  "              which addresses the specific properties of DNA       \n"
  "              sequences (Inverted repeats) is used or not. The     \n"
  "              number 1 turns ON the sub-program using at the same  \n"
  "              time the regular context model. The number 0 does    \n"
  "              not contemple its use (Inverted repeats OFF). The    \n"
  "              use of this sub-program increases the necessary time \n"
  "              to compress but it does not affect the RAM.          \n"
  "      [NB_G]: (real [0;1)) real number to define gamma. This value \n"
  "              represents the decayment forgetting factor of the    \n"
  "              regular context model in definition.                 \n"
  "      [NB_S]: (integer [0;20]) maximum number of editions allowed  \n"
  "              to use a substitutional tolerant model with the same \n"
  "              memory model of the regular context model with       \n"
  "              order size equal to [NB_C]. The value 0 stands for   \n"
  "              turning the tolerant context model off. When the     \n"
  "              model is on, it pauses when the number of editions   \n"
  "              is higher that [NB_C], while it is turned on when    \n"
  "              a complete match of size [NB_C] is seen again. This  \n"
  "              is probabilistic-algorithmic model very usefull to   \n"
  "              handle the high substitutional nature of genomic     \n"
  "              sequences. When [NB_S] > 0, the compressor used more \n"
  "              processing time, but uses the same RAM and, usually, \n"
  "              achieves a substantial higher compression ratio. The \n"
  "              impact of this model is usually only noticed for     \n"
  "              [NB_C] >= 14.                                        \n"
  "      [NB_R]: (integer {0,1}) number to define if a sub-program    \n"
  "              which addresses the specific properties of DNA       \n"
  "              sequences (Inverted repeats) is used or not. It is   \n"
  "              similar to the [NR_I] but for tolerant models.       \n"
  "      [NB_E]: (integer [1;5000]) denominator to build alpha for    \n"
  "              substitutional tolerant context model. It is         \n"
  "              analogous to [NB_D], however to be only used in the  \n"
  "              probabilistic model for computing the statistics of  \n"
  "              the substitutional tolerant context model.           \n"
  "      [NB_A]: (real [0;1)) real number to define gamma. This value \n"
  "              represents the decayment forgetting factor of the    \n"
  "              substitutional tolerant context model in definition. \n"
  "              Its definition and use is analogus to [NB_G].        \n"
  "                                                                   \n"
  "      ... (you may use several context models)                     \n"
  "                                                                   \n"
  "                                                                   \n"
  "      -rm [NB_R]:[NB_C]:[NB_A]:[NB_B]:[NB_L]:[NB_G]:[NB_I]         \n"
  "      Template of a repeat model.                                  \n"
  "      Parameters:                                                  \n"
  "      [NB_R]: (integer [1;10000] maximum number of repeat models   \n"
  "              for the class. On very repetive sequences the RAM    \n"
  "              increases along with this value, however it also     \n"
  "              improves the compression capability.                 \n"
  "      [NB_C]: (integer [1;20]) order size of the repeat context    \n"
  "              model. Higher values use more RAM but, usually, are  \n"
  "              related to a better compression score.               \n"
  "      [NB_A]: (real (0;1]) alpha is a real value, which is a       \n"
  "              parameter estimator. Higher values are usually used  \n"
  "              in lower [NB_C]. When [NB_A] is one, the             \n"
  "              probabilities assume a Laplacian distribution.       \n"
  "      [NB_B]: (real (0;1]) beta is a real value, which is a        \n"
  "              parameter for discarding or maintaining a certain    \n"
  "              repeat model.                                        \n"
  "      [NB_L]: (integer (1;20]) a limit threshold to play with      \n"
  "              [NB_B]. It accepts or not a certain repeat model.    \n"
  "      [NB_G]: (real [0;1)) real number to define gamma. This value \n"
  "              represents the decayment forgetting factor of the    \n"
  "              regular context model in definition.                 \n"
  "      [NB_I]: (integer {0,1}) number to define if a sub-program    \n"
  "              which addresses the specific properties of DNA       \n"
  "              sequences (Inverted repeats) is used or not. The     \n"
  "              number 1 turns ON the sub-program using at the same  \n"
  "              time the regular context model. The number 0 does    \n"
  "              not contemple its use (Inverted repeats OFF). The    \n"
  "              use of this sub-program increases the necessary time \n"
  "              to compress but it does not affect the RAM.          \n"
  "                                                                   \n");
  } 

void PrintMenu(void){
  fprintf(stderr,
  "                                                                       \n"
  "              ██╗ █████╗ ██████╗ ██╗   ██╗██╗███████╗                  \n"
  "              ██║██╔══██╗██╔══██╗██║   ██║██║██╔════╝                  \n"
  "              ██║███████║██████╔╝██║   ██║██║███████╗                  \n"
  "         ██   ██║██╔══██║██╔══██╗╚██╗ ██╔╝██║╚════██║                  \n"
  "         ╚█████╔╝██║  ██║██║  ██║ ╚████╔╝ ██║███████║                  \n"
  "          ╚════╝ ╚═╝  ╚═╝╚═╝  ╚═╝  ╚═══╝  ╚═╝╚══════╝                  \n"
  "                                                                       \n"
  "NAME                                                                   \n"
  "      JARVIS v%u.%u,                                                   \n"
  "      extreme lossless compression of genomic sequences.               \n"
  "                                                                       \n"
  "AUTHORS                                                                \n"
  "      Diogo Pratas        pratas@ua.pt                                 \n"
  "      Morteza Hosseini    seyedmorteza@ua.pt                           \n"
  "      Armando J. Pinho    ap@ua.pt                                     \n"
  "                                                                       \n"
  "SYNOPSIS                                                               \n"
  "      ./JARVIS [OPTION]... [FILE]                                      \n"
  "                                                                       \n"
  "SAMPLE                                                                 \n"
  "      Run Compression         :  ./JARVIS -v -l 4 sequence.txt         \n"
  "      Run Decompression       :  ./JARVIS -v -d sequence.txt.jc        \n"
  "                                                                       \n"
  "DESCRIPTION                                                            \n"
  "      Compress and decompress lossless genomic sequences for           \n"
  "      storage and analysis purposes.                                   \n"
  "      Measure an upper bound of the sequence entropy.                  \n"
  "                                                                       \n"
  "      -h,  --help                                                      \n"
  "           usage guide (help menu).                                    \n"
  "                                                                       \n"
  "      -V,  --version                                                   \n"
  "           Display program and version information.                    \n"
  "                                                                       \n"
  "      -F,  --force                                                     \n"
  "           force mode. Overwrites old files.                           \n"
  "                                                                       \n"
  "      -v,  --verbose                                                   \n"
  "           verbose mode (more information).                            \n"
  "                                                                       \n"
  "      -e,  --estimation                                                \n"
  "           creates [sequence].info with complexity profile.            \n"
  "                                                                       \n"
  "      -s,  --show-levels                                               \n"
  "           show pre-computed compression levels (configured).          \n"
  "                                                                       \n",
  VERSION, RELEASE);

  #ifdef ESTIMATE
  fprintf(stderr,
  "      -e,  --estimate                                                  \n"
  "           it creates a file with the extension \".iae\" with the      \n"
  "           respective information content. If the file is FASTA or     \n"
  "           FASTQ it will only use the \"ACGT\" (genomic) sequence.     \n"
  "                                                                       \n");
  #endif
  
  fprintf(stderr,
  "      -l [NUMBER],  --level [NUMBER]                                   \n"
  "           Compression level (integer).                                \n"
  "           Default level: %u.                                          \n"
  "           It defines compressibility in balance with computational    \n"
  "           resources (RAM & time). Use -s for levels perception.       \n",
  DEFAULT_LEVEL);

  ModelsExplanation();

  fprintf(stderr,
  "      -z [NUMBER],  --selection [NUMBER]                               \n"
  "           Size of the context selection model (integer).              \n"
  "           Default context selection: %u.                              \n"
  "                                                                       \n"
  "      [FILE]                                                           \n"
  "           Input sequence filename (to compress) -- MANDATORY.         \n"
  "           File to compress is the last argument.                      \n"
  "                                                                       \n"
  "COPYRIGHT                                                              \n"
  "      Copyright (C) 2014-2019, IEETA, University of Aveiro.            \n"
  "      This is a Free software, under GPLv3. You may redistribute       \n"
  "      copies of it under the terms of the GNU - General Public         \n"
  "      License v3 <http://www.gnu.org/licenses/gpl.html>. There         \n"
  "      is NOT ANY WARRANTY, to the extent permitted by law.             \n"
  "                                                                       \n",
  DEFAULT_SELECTION);
  }


void PrintVersion(void){
  fprintf(stderr,
  "                                                                       \n"
  "                          ================                             \n"
  "                          | JARVIS  v%u.%u |                        \n"
  "                          ================                             \n"
  "                                                                       \n"
  "   Extreme lossless compression and decompression of DNA sequences.    \n"
  "                                                                       \n"
  "Copyright (C) 2014-2019 University of Aveiro. This is a Free software. \n"
  "You may redistribute copies of it under the terms of the GNU - General \n"
  "Public License v3 <http://www.gnu.org/licenses/gpl.html>. There is NOT \n"
  "ANY WARRANTY, to the extent permitted by law. Developed and written by \n"
  "Diogo Pratas, Morteza Hosseini and Armando J. Pinho.\n\n", VERSION, RELEASE);
  }

void MsgNoModels(void){
  fprintf(stderr, "Error: at least you need to use a context or repeat model!\n");
  }

void FailModelScheme(void){
  fprintf(stderr, "Error: unknown scheme for repeat model arguments!\n");
  fprintf(stderr, "Plz, reset the models according to the description:\n");
  ModelsExplanation();
  }

