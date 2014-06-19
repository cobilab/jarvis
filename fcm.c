/*------------------------------------------------------------------------------

Copyright 2005-2011 Armando J. Pinho (ap@ua.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@ua.pt. The copyright notice above
and this statement of conditions must remain an integral part of each
and every copy made of these files.

------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "defs.h"
#include "mem.h"
#include "fcm.h"

/*----------------------------------------------------------------------------*/

static HCCounters zeroCounters = {0x00, 0x00, 0x00, 0x00};
static HCCounters auxCounters;

static void InitCHashTable(CModel *cModel)

	{ 
	if((cModel->hTable.entries = (CHashTableEntry **)Calloc(cModel->hTable.size,
	  sizeof(CHashTableEntry *))) == NULL)
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if((cModel->hTable.counters = (HCCounters **)Calloc(cModel->hTable.size,
	  sizeof(HCCounters *))) == NULL)
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if((cModel->hTable.entrySize = (unsigned short *)Calloc(cModel->hTable.size,
	  sizeof(unsigned short))) == NULL)
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	cModel->hTable.nUsedEntries = 0;
	cModel->hTable.nUsedKeys = 0;
	}

/*----------------------------------------------------------------------------*/

static void InitCArray(CModel *cModel)

	{
	if(!(cModel->array.counters = (ACCounter *)Calloc(cModel->nPModels *
	  cModel->nSymbols, sizeof(ACCounter))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	}

/*----------------------------------------------------------------------------*/

static void InsertCHashTableKey(CHashTable *hTable, unsigned hIndex,
  unsigned key)

	{
	if(!(hTable->entries[hIndex] = (CHashTableEntry *)
	  Realloc(hTable->entries[hIndex], (hTable->entrySize[hIndex] + 1) *
	  sizeof(CHashTableEntry), sizeof(CHashTableEntry))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if(!hTable->entrySize[hIndex])
		hTable->nUsedEntries++;

	hTable->nUsedKeys++;
	hTable->entries[hIndex][hTable->entrySize[hIndex]].key = key;
	hTable->entrySize[hIndex]++;
	}

/*----------------------------------------------------------------------------*/

static void InsertCCounters(CHashTable *hTable, unsigned hIndex,
  unsigned nHCCounters, unsigned k, unsigned smallCounters)

	{
	if(!(hTable->counters[hIndex] = (HCCounters *)Realloc(hTable->
	  counters[hIndex], (nHCCounters + 1) * sizeof(HCCounters),
	  sizeof(HCCounters))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if(k < nHCCounters)
		memmove(hTable->counters[hIndex][k + 1], hTable->counters[hIndex][k],
		  (nHCCounters - k) * sizeof(HCCounters));

	hTable->counters[hIndex][k][0] = smallCounters & 0x03;
	hTable->counters[hIndex][k][1] = (smallCounters & (0x03 << 2)) >> 2;
	hTable->counters[hIndex][k][2] = (smallCounters & (0x03 << 4)) >> 4;
	hTable->counters[hIndex][k][3] = (smallCounters & (0x03 << 6)) >> 6;
	}

/*----------------------------------------------------------------------------*/

static HCCounter *GetHCCounters(CHashTable *hTable, unsigned key)

	{
	unsigned k = 0, n;
	unsigned hIndex = key % hTable->size; /* The hash index */

	for(n = 0 ; n < hTable->entrySize[hIndex] ; n++)
		{
		/* If key found */
		if(hTable->entries[hIndex][n].key == key)
			{
			switch(hTable->entries[hIndex][n].counters)
				{
				case 0:
					return hTable->counters[hIndex][k];

				default:
					auxCounters[0] = hTable->entries[hIndex][n].counters &
					  0x03;
					auxCounters[1] = (hTable->entries[hIndex][n].counters &
					  (0x03 << 2)) >> 2;
					auxCounters[2] = (hTable->entries[hIndex][n].counters &
					  (0x03 << 4)) >> 4;
					auxCounters[3] = (hTable->entries[hIndex][n].counters &
					  (0x03 << 6)) >> 6;
					return auxCounters;

				}

			}

		if(hTable->entries[hIndex][n].counters == 0)
			k++;

		}

	return NULL;
	}

/*----------------------------------------------------------------------------*/

PModel *CreatePModel(unsigned nSymbols)

	{
	PModel *pModel;

	if(!(pModel = (PModel *)Malloc(sizeof(PModel))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if(!(pModel->freqs = (unsigned *)Malloc(nSymbols * sizeof(unsigned))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	return pModel;
	}

/*----------------------------------------------------------------------------*/

FloatPModel *CreateFloatPModel(unsigned nSymbols)

	{
	FloatPModel *floatPModel;

	if(!(floatPModel = (FloatPModel *)Malloc(sizeof(FloatPModel))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if(!(floatPModel->freqs = (double *)Malloc(nSymbols * sizeof(double))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	return floatPModel;
	}

/*----------------------------------------------------------------------------*/

void UpdateCModelCounter(CModel *cModel, unsigned pModelIdx, unsigned symbol)

	{
	unsigned n;
	ACCounter *aCounters;

	if(cModel->mode == FCM_HASH_TABLE_MODE)
		{
		unsigned char smallCounter;
		unsigned i, k = 0;
		unsigned nHCCounters; /* The number of HCCounters in this entry */
		unsigned hIndex = pModelIdx % cModel->hTable.size; /* The hash index */

		for(n = 0 ; n < cModel->hTable.entrySize[hIndex] ; n++)
			{
			/* If key found */
			if(cModel->hTable.entries[hIndex][n].key == pModelIdx)
				{
				/*
				 * If "counters" is zero, then update the "large" counters.
				 */
				if(cModel->hTable.entries[hIndex][n].counters == 0)
					{
					if(++cModel->hTable.counters[hIndex][k][symbol] == 255)
						for(i = 0 ; i < cModel->nSymbols ; i++)
							cModel->hTable.counters[hIndex][k][i] >>= 1;

					return;
					}
				
				smallCounter = (cModel->hTable.entries[hIndex][n].counters >>
				  (symbol << 1)) & 0x03;
				/*
				 * If "counters" is non-zero, then this is at least the
				 * second time that this key is generated. Therefore,
				 * if the "small" counter of the symbol if full (i.e.,
				 * is equal to 3), then the "large" counters have to be
				 * inserted into the right position.
				 */
				if(smallCounter == 3)
					{
					nHCCounters = k;
					for(i = n + 1 ; i < cModel->hTable.entrySize[hIndex] ; i++)
						if(cModel->hTable.entries[hIndex][i].counters == 0)
							nHCCounters++;

					InsertCCounters(&cModel->hTable, hIndex, nHCCounters, k,
					  cModel->hTable.entries[hIndex][n].counters);
					cModel->hTable.entries[hIndex][n].counters = 0;
					cModel->hTable.counters[hIndex][k][symbol]++;
					return;
					}

				/*
				 * There is still room for incrementing the "small" counter.
				 */
				else
					{
					smallCounter++;
					cModel->hTable.entries[hIndex][n].counters &=
					  ~(0x03 << (symbol << 1));
					cModel->hTable.entries[hIndex][n].counters |=
					  (smallCounter << (symbol << 1));
					return;
					}

				}

			/* Keeps counting the number of HCCounters in this entry */
			if(!cModel->hTable.entries[hIndex][n].counters)
				k++;

			}

		/* If key not found */
		InsertCHashTableKey(&cModel->hTable, hIndex, pModelIdx);
		cModel->hTable.entries[hIndex][cModel->hTable.entrySize[hIndex] - 1].
		  counters = (0x01 << (symbol << 1));
		}

	else
		{
		aCounters = &cModel->array.counters[pModelIdx * cModel->nSymbols];
		aCounters[symbol]++;
		if(aCounters[symbol] == cModel->maxCount && cModel->maxCount != 0)
			for(n = 0 ; n < cModel->nSymbols ; n++)
				aCounters[n] >>= 1;

		}

	}

/*----------------------------------------------------------------------------*/

CModel *CreateCModel(unsigned ctxSize, unsigned nSymbols, unsigned alphaNum,
  unsigned alphaDen, unsigned maxCount, unsigned hSize, double gamma, char ir)

	{
	CModel *cModel;

	if(!(cModel = (CModel *)Calloc(1, sizeof(CModel))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if(ctxSize > 16)
		{
		fprintf(stderr, "Error: context size cannot be greater than 16\n");
		exit(1);
		}

	cModel->nPModels = (ULL)pow(nSymbols, ctxSize);
	cModel->ctxSize = ctxSize;
	cModel->nSymbols = nSymbols;
	cModel->alphaNum = alphaNum;
	cModel->alphaDen = alphaDen;
	cModel->weight = 1;
	cModel->gamma = gamma;
	cModel->ir = ir;
	cModel->hTable.size = hSize;
	if((ULL)(cModel->nPModels) * nSymbols * sizeof(ACCounter) >> 20 >
	  FCM_MAX_ARRAY_MEMORY)
		{
		cModel->mode = FCM_HASH_TABLE_MODE;
		cModel->maxCount = maxCount >> 8;
		InitCHashTable(cModel);
		}

	else
		{
		cModel->mode = FCM_ARRAY_MODE;
		cModel->maxCount = maxCount;
		InitCArray(cModel);
		}

	return cModel;
	}

/*----------------------------------------------------------------------------*/

void ComputePModel(CModel *cModel, PModel *pModel, unsigned pModelIdx)

	{
	int symbol;
	ACCounter *aCounters;
	HCCounter *hCounters;

	pModel->sum = 0;
	if(cModel->mode == FCM_HASH_TABLE_MODE)
		{
		if(!(hCounters = GetHCCounters(&cModel->hTable, pModelIdx)))
			hCounters = zeroCounters;

		for(symbol = 0 ; symbol < cModel->nSymbols ; symbol++)
			{
			pModel->freqs[symbol] = cModel->alphaNum + cModel->alphaDen *
			  hCounters[symbol];
			pModel->sum += pModel->freqs[symbol];
			}
		}

	else
		{
		aCounters = &cModel->array.counters[pModelIdx * cModel->nSymbols];
		for(symbol = 0 ; symbol < cModel->nSymbols ; symbol++)
			{
			pModel->freqs[symbol] = cModel->alphaNum + cModel->alphaDen *
			  aCounters[symbol];
			pModel->sum += pModel->freqs[symbol];
			}
		}

	}

/*----------------------------------------------------------------------------*/

double PModelSymbolNats(PModel *pModel, unsigned symbol)

	{
	return log((double)pModel->sum / pModel->freqs[symbol]);
	}

/*----------------------------------------------------------------------------*/

double FractionOfPModelsUsed(CModel *cModel)

	{
	unsigned pModel, symbol, sum, counter = 0;
	ACCounter *aCounters;
	HCCounter *hCounters;

	sum = 0;
	for(pModel = 0 ; pModel < cModel->nPModels ; pModel++)
		{
		if(cModel->mode == FCM_HASH_TABLE_MODE)
			{
			hCounters = GetHCCounters(&(cModel->hTable), pModel);
			if(hCounters)
				counter++;

			}

		else
			{
			aCounters = &(cModel->array.counters[pModel * cModel->nSymbols]);
			for(symbol = 0 ; symbol < cModel->nSymbols ; symbol++)
				sum += aCounters[symbol];

			if(sum != 0)
				counter++;

			}

		}

	return (double)counter / cModel->nPModels;
	}

/*----------------------------------------------------------------------------*/

double FractionOfPModelsUsedOnce(CModel *cModel)

	{
	unsigned pModelIdx;
	unsigned symbol, sum, counter = 0;
	ACCounter *aCounters;
	HCCounter *hCounters;

	sum = 0;
	for(pModelIdx = 0 ; pModelIdx < cModel->nPModels ; pModelIdx++)
		{
		if(cModel->mode == FCM_HASH_TABLE_MODE)
			{
			hCounters = GetHCCounters(&(cModel->hTable), pModelIdx);

			if(!hCounters)
				continue;

			for(symbol = 0 ; symbol < cModel->nSymbols ; symbol++)
				sum += hCounters[symbol];

			if(sum == 1)
				counter++;

			}

		else
			{
			aCounters = &(cModel->array.counters[pModelIdx * cModel->nSymbols]);
			for(symbol = 0 ; symbol < cModel->nSymbols ; symbol++)
				sum += aCounters[symbol];

			if(sum == 1)
				counter++;

			}

		}

	return (double)counter / cModel->nPModels;
	}

/*----------------------------------------------------------------------------*/

void CHashingStats(CModel *cModel)

	{
	unsigned entry, n, k, emptyEntries = 0, nSmallCounters = 0,
	  maxEntrySize = 0;
	ULL possibleKeys;
	double average = (double)cModel->hTable.nUsedKeys / cModel->hTable.size;
	double deviation = 0;

	for(entry = 0 ; entry < cModel->hTable.size ; entry++)
		{
		deviation += fabs(average - cModel->hTable.entrySize[entry]);

		if(!cModel->hTable.entrySize[entry])
			{
			emptyEntries++;
			continue;
			}

		if(cModel->hTable.entrySize[entry] > maxEntrySize)
			maxEntrySize = cModel->hTable.entrySize[entry];

		k = 0;
		for(n = 0 ; n < cModel->hTable.entrySize[entry] ; n++)
			{
			/*
			 * If "counters" is non-zero, then this key did not require
			 * "large" counters
			 */
			if(cModel->hTable.entries[entry][n].counters != 0)
				nSmallCounters++;

			/*
			 * Keeps counting the number of HCCounters in this entry.
			 * counters = 0 means that the corresponding key has been
			 * generated more than once and, therefore, the set of
			 * counters has been allocated.
			 */
			if(cModel->hTable.entries[entry][n].counters == 0)
				k++;

			}

		}

	possibleKeys = powl((double)cModel->nSymbols, (double)cModel->ctxSize);

	printf("Hash size ......... %u\n", cModel->hTable.size);
	printf("Used entries ...... %u\n", cModel->hTable.nUsedEntries);
	printf("Ideal entry size .. %.1f\n", average);
	printf("Deviation ......... %.1f\n", deviation / cModel->hTable.size);
	printf("Used keys ......... %u [%.2f %% of %llu]\n",
	  cModel->hTable.nUsedKeys, 100.0 * (double)cModel->hTable.nUsedKeys /
	  possibleKeys, possibleKeys);
	printf("Small counters .... %u\n", nSmallCounters);
	printf("Large counters .... %u\n", cModel->hTable.nUsedKeys-nSmallCounters);
	printf("Max entry size .... %u\n", maxEntrySize);
	}

