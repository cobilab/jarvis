/*------------------------------------------------------------------------------

Copyright 2005-2009 Armando J. Pinho (ap@ua.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@ua.pt. The copyright notice above
and this statement of conditions must remain an integral part of each
and every copy made of these files.

------------------------------------------------------------------------------*/

/*
 * Data structures for handling finite-context models.
 */

#ifndef FCM_H_INCLUDED
#define FCM_H_INCLUDED

#define FCM_MAX_ARRAY_MEMORY		1024 /* DNA: size 12 = 128 MB */
#define FCM_ARRAY_MODE				0
#define FCM_HASH_TABLE_MODE			1

typedef struct
	{
	unsigned		key;		/* The key stored in this entry */
	unsigned char	counters;	/* "Small" counters: 2 bits for each one */
	}
CHashTableEntry;

typedef struct
	{
	unsigned		size;		/* Size of the hash table */
	unsigned short	*entrySize;	/* Number of keys in this entry */
	CHashTableEntry	**entries;	/* The heads of the hash table lists */
	HCCounters		**counters;	/* The context counters */
	unsigned		nUsedEntries;
	unsigned		nUsedKeys;
	}
CHashTable;

typedef struct
	{
	ACCounter	*counters;
	}
CArray;

typedef struct
	{
	unsigned	*freqs;
	unsigned	sum;
	}
PModel;

typedef struct
	{
	double		*freqs;
	}
FloatPModel;

typedef struct
	{
	unsigned	ctxSize;	/* Current depth of context template */
	unsigned	nSymbols;	/* Number of coding symbols */
	ULL			nPModels;	/* Maximum number of probability models */
	unsigned	alphaNum;	/* Numerator of alpha */
	unsigned	alphaDen;	/* Denominator of alpha */
	unsigned	maxCount;	/* Counters /= 2 if one counter >= maxCount */
	double		weight;		/* Weight of this model in the mixture formula */
	double		gamma;		/* Performance decaying parameter */
	char		ir;			/* If 'y', update the inverted complements */
	unsigned	mode;
	CHashTable	hTable;
	CArray		array;
	}
CModel;

PModel *CreatePModel(unsigned nSymbols);
FloatPModel *CreateFloatPModel(unsigned nSymbols);
void UpdateCModelCounter(CModel *cModel, unsigned pModelIdx, unsigned symbol);
CModel *CreateCModel(unsigned ctxSize, unsigned nSymbols, unsigned alphaNum,
  unsigned alphaDen, unsigned maxCount, unsigned hSize, double gamma, char ir);
double FractionOfPModelsUsed(CModel *cModel);
double FractionOfPModelsUsedOnce(CModel *cModel);
void ComputePModel(CModel *cModel, PModel *pModel, unsigned pModelIdx);
double PModelSymbolNats(PModel *pModel, unsigned symbol);
void CHashingStats(CModel *cModel);

#endif /* FCM_H_INCLUDED */

