/*------------------------------------------------------------------------------

Copyright 2011 Armando J. Pinho (ap@ua.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@ua.pt. The copyright notice above
and this statement of conditions must remain an integral part of each
and every copy made of these files.

------------------------------------------------------------------------------*/

#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#pragma pack(1)

#define N_SYMBOLS 4

typedef unsigned long long ULL;
typedef unsigned char Symbol;
typedef unsigned short ACCounter; /* Size of context counters for arrays */
typedef unsigned char  HCCounter; /* Size of context counters for hash tables */
typedef HCCounter HCCounters[N_SYMBOLS];

typedef struct
	{
	Symbol	*bases;
	int	size;
	int	unknowns;
	int	convert;
	}
Sequence;

#define DEFAULT_PMODEL_ALPHA_NUM		1
#define DEFAULT_PMODEL_ALPHA_DEN		1
#define DEFAULT_PMODEL_MAX_COUNT		((1 << (sizeof(ACCounter) * 8)) - 1)
/* If 0, never divide counters by maxCount */

#define STORAGE_BITS_FILE_SIZE			32
#define STORAGE_BITS_PMODEL_ALPHA_NUM		6
#define STORAGE_BITS_PMODEL_ALPHA_DEN		6
#define STORAGE_BITS_PMODEL_MAX_COUNT		30
#define STORAGE_BITS_N_CMODELS			5
#define STORAGE_BITS_TEMPLATE_SIZE		5
#define STORAGE_BITS_TEMPLATE_POSITION		5
#define STORAGE_BITS_INVERTED_REPEATS		1
#define STORAGE_BITS_POWER_FUNCTION		1
#define STORAGE_BITS_HSIZE			30
#define STORAGE_BITS_GAMMA			16

#endif /* DEFS_H_INCLUDED */

