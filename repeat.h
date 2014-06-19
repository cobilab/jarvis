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

/*
 * Data structures for handling the repeat models.
 */

#ifndef REPEAT_H_INCLUDED
#define REPEAT_H_INCLUDED

typedef struct
	{
	unsigned	key;	/* The key stored in this entry */
	unsigned	*pos;	/* List of repeating positions */
	unsigned	nPos;
	}
RHTableEntry;

typedef struct
	{
	unsigned	size;		/* Size of the hash table */
	unsigned	keySize;	/* Size of the indexing key */
	unsigned short	*entrySize;	/* Number of keys in this entry */
	RHTableEntry	**entries;	/* The heads of the hash table lists */
	unsigned	nUsedEntries;
	unsigned	nUsedKeys;
	}
RHTable;

typedef struct
	{
	unsigned	pos;			/* Position of the next predicted symbol */
	unsigned	nHits;			/* Number of times that the model was right */
	unsigned	nTries;			/* Number of times the model was used */
	char		ic;			/* If 'y', it is an inverted complement */
	double		probs[N_SYMBOLS];	/* Symbol probabilities given by this model */
	double		weight;			/* Weight of this model in the mixture formula*/
	double		alpha;			/* Alpha of the probability estimator */
	double		gamma;			/* Performance decaying parameter */
	double		lastHit;		/* Performance decaying parameter */
	double		acting;			/* Performance decaying parameter */	
	}
RModel;

typedef struct
	{
	double		initWeight;	/* Initial weight for these models */
	double		initAlpha;	/* Initial alpha */
	double		initGamma;	/* Initial performance decaying parameter */
	double		beta;		/* repeat performance decaying parameter for repeat remove */
	unsigned	limit;		/* repeat performance limit - associated with beta */
	char		allowIC;	/* If 'y', allow inverted complements */
	unsigned	maxRModels;	/* The maximum number of repeat models */
	unsigned	nRModels;	/* The current number of repeat models */
	RModel		*rModels;	/* The repeat models */
	RHTable		*hTable;	/* Hash-table of repeating kmers */
	}
RClass;


int StartRModel(RClass *rClass, unsigned rModel, unsigned templateIdx, char ic);
RClass *CreateRClass(unsigned maxRModels, unsigned keySize, unsigned hSize,
  double initWeight, double initAlpha, double initGamma, double beta, 
    unsigned limit, char allowIC);
void InsertKmerPos(RClass *rClass, unsigned key, unsigned pos);
void RClassHashingStats(RClass *rClass);
RHTableEntry *GetRHTableEntry(RClass *rClass, unsigned key);
void ComputeRModelProbs(RModel *rModel, Symbol *seq);
void UpdateRModel(RModel *rModel, Symbol *seq, Symbol symbol);

#endif /* REPEAT_H_INCLUDED */

