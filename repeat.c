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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "defs.h"
#include "mem.h"
#include "repeat.h"

/*----------------------------------------------------------------------------*/

int StartRModel(RClass *rClass, unsigned rModel, unsigned templateIdx, char ic)

	{
	unsigned s, pos;
	RHTableEntry *rHTableEntry;

	if((rHTableEntry = GetRHTableEntry(rClass, templateIdx)) == NULL)
		return 0;

	pos = random() % rHTableEntry->nPos;
	if(ic == 'n')
		//rClass->rModels[rModel].pos = rHTableEntry->pos[rHTableEntry->nPos - 1];
		//rClass->rModels[rModel].pos = rHTableEntry->pos[0];
		rClass->rModels[rModel].pos = rHTableEntry->pos[pos];

	else
		{
		//if(rHTableEntry->pos[rHTableEntry->nPos - 1] <=
		 // rClass->hTable->keySize + 1)
		//if(rHTableEntry->pos[0] <= rClass->hTable->keySize + 1)
		if(rHTableEntry->pos[pos] <= rClass->hTable->keySize + 1)
			return 0;

		//rClass->rModels[rModel].pos =
		 // rHTableEntry->pos[rHTableEntry->nPos - 1] -
		 // rClass->hTable->keySize - 1;
		//rClass->rModels[rModel].pos = rHTableEntry->pos[0] -
		 // rClass->hTable->keySize - 1;
		rClass->rModels[rModel].pos = rHTableEntry->pos[pos] -
		  rClass->hTable->keySize - 1;

		}

	rClass->rModels[rModel].nHits = 0;
	rClass->rModels[rModel].nTries = 0;
	rClass->rModels[rModel].ic = ic;
	rClass->rModels[rModel].acting = 0;
	rClass->rModels[rModel].weight = rClass->initWeight;
	rClass->rModels[rModel].gamma = rClass->initGamma;
	rClass->rModels[rModel].alpha = rClass->initAlpha;

	for(s = 0 ; s < N_SYMBOLS ; s++)
		rClass->rModels[rModel].probs[s] = 0;

	return 1;
	}

/*----------------------------------------------------------------------------*/

RClass *CreateRClass(unsigned maxRModels, unsigned keySize, unsigned hSize,
  double initWeight, double initAlpha, double initGamma, double beta, 
    unsigned limit, char allowIC)
	{

	RClass *rClass;

	if(!(rClass = (RClass *)Calloc(1, sizeof(RClass))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if(!(rClass->hTable = (RHTable *)Calloc(1, sizeof(RHTable))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if((rClass->hTable->entries = (RHTableEntry **)Calloc(hSize,
	  sizeof(RHTableEntry *))) == NULL)
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if((rClass->hTable->entrySize = (unsigned short *)Calloc(hSize,
	  sizeof(unsigned short))) == NULL)
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if(!(rClass->rModels = (RModel *)Calloc(maxRModels, sizeof(RModel))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	rClass->maxRModels = maxRModels;
	rClass->initAlpha = initAlpha;
	rClass->initWeight = initWeight;
	rClass->initGamma = initGamma;
	rClass->beta = beta;
	rClass->limit = limit;
	rClass->allowIC = allowIC;
	rClass->hTable->size = hSize;
	rClass->hTable->keySize = keySize;
	rClass->hTable->nUsedEntries = 0;
	rClass->hTable->nUsedKeys = 0;

	return rClass;
	}

/*------------------------------------------------------------------------------

	"pos" points to the first symbol after the kmer.

------------------------------------------------------------------------------*/

void InsertKmerPos(RClass *rClass, unsigned key, unsigned pos)

	{
	unsigned n;
	unsigned hIdx = key % rClass->hTable->size; /* The hash index */

	for(n = 0 ; n < rClass->hTable->entrySize[hIdx] ; n++)
		{
		/* If key found */
		if(rClass->hTable->entries[hIdx][n].key == key)
			{
			/* Create a new position */
			if(!(rClass->hTable->entries[hIdx][n].pos = (unsigned *)
			  Realloc(rClass->hTable->entries[hIdx][n].pos,
			  (rClass->hTable->entries[hIdx][n].nPos + 1) *
			  sizeof(unsigned), sizeof(unsigned))))
				{
				fprintf(stderr, "Error: in memory allocation\n");
				exit(1);
				}

			rClass->hTable->entries[hIdx][n].
			  pos[rClass->hTable->entries[hIdx][n].nPos++] = pos;

			return;
			}

		}

	/* If key not found, create a new entry */
	if(!(rClass->hTable->entries[hIdx] =
	  (RHTableEntry *)Realloc(rClass->hTable->entries[hIdx],
	  (rClass->hTable->entrySize[hIdx] + 1) * sizeof(RHTableEntry),
	  sizeof(RHTableEntry))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	/* Create a new position */
	if(!(rClass->hTable->entries[hIdx][rClass->hTable->entrySize[hIdx]].pos =
	  (unsigned *)Malloc(sizeof(unsigned))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	rClass->hTable->entries[hIdx][rClass->hTable->entrySize[hIdx]].nPos = 1;
	rClass->hTable->entries[hIdx][rClass->hTable->entrySize[hIdx]].key = key;
	rClass->hTable->entries[hIdx][rClass->hTable->entrySize[hIdx]].pos[0] = pos;

	if(!rClass->hTable->entrySize[hIdx])
		rClass->hTable->nUsedEntries++;

	rClass->hTable->nUsedKeys++;
	rClass->hTable->entrySize[hIdx]++;
	}

/*----------------------------------------------------------------------------*/

void RClassHashingStats(RClass *rClass)

	{
	unsigned entry, emptyEntries = 0, maxEntrySize = 0;
	ULL possibleKeys;
	double average = (double)rClass->hTable->nUsedKeys / rClass->hTable->size;
	double deviation = 0;

	for(entry = 0 ; entry < rClass->hTable->size ; entry++)
		{
		deviation += fabs(average - rClass->hTable->entrySize[entry]);

		if(!rClass->hTable->entrySize[entry])
			{
			emptyEntries++;
			continue;
			}

		if(rClass->hTable->entrySize[entry] > maxEntrySize)
			maxEntrySize = rClass->hTable->entrySize[entry];

		}

	possibleKeys = powl((double)N_SYMBOLS,(double)rClass->hTable->keySize);

	printf("Hash size ......... %u\n", rClass->hTable->size);
	printf("Used entries ...... %u\n", rClass->hTable->nUsedEntries);
	printf("Ideal entry size .. %.1f\n", average);
	printf("Deviation ......... %.1f\n", deviation / rClass->hTable->size);
	printf("Used keys ......... %u [%.2f %% of %llu]\n",
	  rClass->hTable->nUsedKeys, 100.0 *
	  (double)rClass->hTable->nUsedKeys / possibleKeys, possibleKeys);
	printf("Max entry size .... %u\n", maxEntrySize);
	}

/*----------------------------------------------------------------------------*/

RHTableEntry *GetRHTableEntry(RClass *rClass, unsigned key)

	{
	unsigned n;
	unsigned hIdx = key % rClass->hTable->size; /* The hash index */

	for(n = 0 ; n < rClass->hTable->entrySize[hIdx] ; n++)
		if(rClass->hTable->entries[hIdx][n].key == key)
			return &rClass->hTable->entries[hIdx][n];

	return NULL;
	}

/*----------------------------------------------------------------------------*/

void ComputeRModelProbs(RModel *rModel, Symbol *seq)

	{
	Symbol sym, rSym;

	if(rModel->ic == 'y')
		rSym = 3 - seq[rModel->pos];

	else
		rSym = seq[rModel->pos];

	rModel->probs[rSym] = (rModel->nHits + rModel->alpha) /
	  (rModel->nTries + 2 * rModel->alpha);

	for(sym = 0 ; sym < N_SYMBOLS ; sym++)
		if(sym != rSym)
			rModel->probs[sym] = (1 - rModel->probs[rSym]) / 3;

	}

/*----------------------------------------------------------------------------*/

void UpdateRModel(RModel *rModel, Symbol *seq, Symbol sym)

	{
	rModel->lastHit = 1;
	if(rModel->ic == 'n')
		{
		if(seq[rModel->pos++] == sym)
			{
			rModel->nHits++;
			rModel->lastHit = 0;
			}
		}

	else
		{
		if(3 - seq[rModel->pos--] == sym)
			{
			rModel->nHits++;
			rModel->lastHit = 0;
			}
		}

	rModel->nTries++;
	}

